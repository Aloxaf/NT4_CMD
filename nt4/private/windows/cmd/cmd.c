#include "cmd.h"


//
// Used in rebuilding command lines for display
//
#define NSPC    0                                                               // Don't use space
#define YSPC    1                                                               // Do use space

extern CPINFO CurrentCPInfo;
extern UINT   CurrentCP;
extern ULONG  LastMsgNo;
//
// Jump buffers used to return to main loop after some error condition
//
jmp_buf MainEnv ;               // SigHand() uses to return to main
jmp_buf CmdJBuf1 ;              // Both of these buffers are used by
jmp_buf CmdJBuf2 ;              // various parts of Command for error

//
// rioCur points to a linked list of rio structures dynamically
// allocated when redirection is performed.  Note that memory is automatically
// freed when Dispatch completed work. rioCur points to last entry.
//
struct rio *rioCur = NULL ;

//
// Retrun code for last external program
//
int LastRetCode ;

//
// Constants used in parsing
//
extern TCHAR PathChar;
extern TCHAR SwitChar;

extern TCHAR Delimiters[];
extern TCHAR Delim2[];

//
// Current Drive:Directory. Set in ChDir
// It it is change temp. SaveDir used used to old original
//
extern TCHAR CurDrvDir[];

//
// Name of NULL device. Used to output to nothing
//
extern TCHAR DevNul[];

//
// flag if we found a label that was target of goto
//
extern BOOLEAN GotoFlag;

//
// Number of elements in Data stack
//
extern ULONG DCount ;

//
// Environment string to locate command shell.
//
extern TCHAR ComSpecStr[] ;

//
// DOS error code
//
extern unsigned DosErr ;

//
// Data for start and executing a batch file. Used in calls
//
extern struct batdata *CurBat ;

//
// Set of /K switch on command line set.
//
extern  BOOLEAN fSingleBatchLine;

//
// Set of /c switch on command line set.
//
extern BOOLEAN fSingleCmdLine;

//
// Alternative path (DDPATH) to search
//
extern TCHAR AppendStr[] ;

//
// flag if control-c was seen
//
extern  BOOL CtrlCSeen;
extern  BOOLEAN fPrintCtrlC;

extern PTCHAR    pszTitleCur;
extern BOOLEAN  fTitleChanged;


//
// Prototypes
//

PTCHAR
Init();

int
(*GetFuncPtr())() ;

PTCHAR
GetEnvVar() ;

PTCHAR
EatWS() ;

int
UnParse(struct node   *, PTCHAR);

int
UnBuild(struct node   *, PTCHAR);

void
UnDuRd(struct node   *, PTCHAR);

void
SPutC(PTCHAR, PTCHAR,int );

PTCHAR
argstr1();

//
// Used to set and reset ctlcseen flag
//
VOID    SetCtrlC();
VOID    ResetCtrlC();


//
// to monitor stack usage
//
extern BOOLEAN  flChkStack;
extern PVOID    FixedPtrOnStack;

typedef struct {
    PVOID   Base;
    PVOID   GuardPage;
    PVOID   Bottom;
    PVOID   ApprxSP;
} STACK_USE;

extern STACK_USE   GlStackUsage;

extern int ChkStack (PVOID pFixed, STACK_USE *pStackUse);



_CRTAPI1
main()

/*++

Routine Description:

    Main entry point for command interpreter

Arguments:
    Command line:

        /P - Permanent Command.  Set permanent CMD flag.
        /C - Single command.  Build a command line out of the rest of
             the args and pass it back to Init.
        /K - Same as /C but also set fSingleBatchLine flag.
        /Q - No echo

Return Value:

    Return: 0    - If success
            1    - Parsing Error
            0xFF - Could  not init
            n    - Return code from command
--*/

{
    CHAR        VarOnStack;
    struct node *pnodeCmdTree ;

    //
    // When in multi-cmd mode tells parser where to get input from.
    //
    int InputType ;


    //
    // Pointer to current command line
    //
    PTCHAR pszCmdLine;

    //
    // flag used when a setjmp returns while processing /K
    // error and move to next line.
    //
    unsigned fIgnore = FALSE;
    unsigned ReturnCode;

    //
    // flChkStack is turned ON initially here and it stays ON while
    // I believe the information returned by ChkStack() is correct.
    //
    // It is turned OFF the first time I don't believe that info and
    // therefore I don't want to make any decisions changing the CMD's
    // behavior.
    //
    // It will stay OFF until CMD terminates so we will never check
    // stack usage again.
    //
    // I implemented one method to prevent CMD.EXE from the stack overflow:
    // Have count and limit of recursion in batch file processing and check
    // stack every time we exceed the limit of recursion until we reach 90%
    // of stack usage.
    // If (stack usage >= 90% of 1 MByte) then terminate batch file
    // unconditionally and handle such termination properly (freeing memory
    // and stack and saving CMD.EXE)
    //
    // It is also possible to implement SEH but then we won't know about
    // CMD problems.
    //

    flChkStack = 1;

    FixedPtrOnStack = (VOID *) &VarOnStack;     // to be used in ChkStack()

    if ( ChkStack (FixedPtrOnStack, &GlStackUsage) == FAILURE ) {
        flChkStack = 0;
    }


    //
    // Initialize the DBCS lead byte table based on the current locale.
    //

    InitializeDbcsLeadCharTable( );

    //
    // Set base APIs to operate in OEM mode
    //
#ifndef UNICODE
    SetFileApisToOEM();
#endif  /* Unicode */

    SetTEBLangID();

    //
    // Init returns NULL when not in single command mode
    // (i.e. /c or /k)
    //
    if ((pszCmdLine = Init()) != NULL) {
        if (setjmp(MainEnv)) {
            //
            // If processing /K and setjmp'd out of init. then ignore
            //
            if ( fSingleBatchLine ){
                fIgnore = TRUE;
            } else {
                CMDexit(0xff) ;
            }
        }

        if ( !fIgnore ){

            DEBUG((MNGRP, MNLVL, "MAIN: Single command mode on `%ws'", pszCmdLine)) ;

            if ((pnodeCmdTree = Parser(READSTRING, (int)pszCmdLine, DCount)) == (struct node *) PARSERROR)
                CMDexit(MAINERROR) ;

            if (pnodeCmdTree == (struct node *) EOF)
                CMDexit(SUCCESS) ;

            DEBUG((MNGRP, MNLVL, "MAIN: Single command parsed successfully.")) ;
            ReturnCode = Dispatch(RIO_MAIN, pnodeCmdTree);

            //
            // Make sure we have the correct console modes.
            //
            ResetConsoleMode();

#ifdef JAPAN
            //
            // Get current CodePage Info.  We need this to decide whether
            // or not to use half-width characters.
            //
            GetCPInfo((CurrentCP=GetConsoleOutputCP()), &CurrentCPInfo);
            //
            // Maybe console output code page was changed by CHCP or MODE,
            // so need to reset LanguageID to correspond to code page.
            //
            SetTEBLangID();
#endif
            if ( !fSingleBatchLine )
                CMDexit( ReturnCode );

            fSingleBatchLine = FALSE;       // Allow ASync exec of GUI apps now
        }
    }

    //
    // Through init and single command processing. reset our Setjmp location
    // to here for error processing.
    //
    if (ReturnCode = setjmp(MainEnv)) {

        //
        // BUGBUG fix later to have a generalized abort
        //        for not assume this is a real abort from
        //        eof on stdin redirected.

        if (ReturnCode == EXIT_EOF) {
            CMDexit(SUCCESS);
        }
    }

    //
    // Check if our I/O has been redirected. This is used to tell
    // where we should read input from.
    //
    InputType = (FileIsDevice(STDIN)) ? READSTDIN : READFILE ;

    DEBUG((MNGRP,MNLVL,"MAIN: Multi command mode, InputType = %d", InputType)) ;

    //
    // If we are reading from a file, make sure the input mode is binary.
    // CRLF translations screw up the lexer because FillBuf() wants to
    // seek around in the file.
    //
    if(InputType == READFILE) {
        _setmode(STDIN,_O_BINARY);
    }

    //
    // Loop till out of input or error parsing.
    //
    while (TRUE) {

        DEBUG((MNGRP, MNLVL, "MAIN: Calling Parser.")) ;

        GotoFlag = FALSE ;
        ResetCtrlC();

        if ((pnodeCmdTree = Parser(InputType, STDIN, FS_FREEALL)) == (struct node *) PARSERROR) {
            DEBUG((MNGRP, MNLVL, "MAIN: Parse failed.")) ;

        } else if (pnodeCmdTree == (struct node *) EOF)
            CMDexit(SUCCESS) ;

        else {
            ResetCtrlC();
            DEBUG((MNGRP, MNLVL, "MAIN: Parsed OK, DISPATCHing.")) ;
            Dispatch(RIO_MAIN, pnodeCmdTree) ;

            //
            // Make sure we have the correct console modes.
            //
            ResetConsoleMode();

            //
            // Get current CodePage Info.  We need this to decide whether
            // or not to use half-width characters.
            //
            GetCPInfo((CurrentCP=GetConsoleOutputCP()), &CurrentCPInfo);
            //
            // Maybe console output code page was changed by CHCP or MODE,
            // so need to reset LanguageID to correspond to code page.
            //
            SetTEBLangID();

            DEBUG((MNGRP, MNLVL, "MAIN: Dispatch returned.")) ;
        }
    }

    CMDexit(SUCCESS);
    return(SUCCESS);
}


int
Dispatch(
    IN int RioType,
    IN struct node *pnodeCmdTree
    )
/*++

Routine Description:

    Set up any I/O redirection for the current node.  Find out who is
    supposed to process this node and call the routine to do it.  Reset
    stdin/stdout if necessary.

    Dispatch() must now be called with all args present since the RioType
    is needed to properly identify the redirection list element.

    Dispatch() determines the need for redirection by examination of the
    RioType and command node and calls SetRedir only if necessary. Also,
    in like manner, Dispatch() only calls ResetRedir if redirection was
    actually performed.

    The conditional that determines whether newline will be issued
    following commands (prior to prompt), had to be altered so that the
    execution of piped commands did not each issue a newline.  The pre-
    prompt newline for pipe series is now issued by ePipe().

Arguments:

    RioType     - tells SetRedir the routine responsible for redirection
    pnodeCmdTree        - the root of the parse tree to be executed

Return Value:

    The return code from the command/function that was executed or
    FAILURE if redirection error.

--*/

{


    int comretcode ;                // Retcode of the cmnd executed
    struct cmdnode *pcmdnode ;      // pointer to current command node
    PTCHAR pbCmdBuf ;               // Buffer used in building command


    DEBUG((MNGRP, DPLVL, "DISP: pnodeCmdTree = 0x%04x, RioType = %d", pnodeCmdTree, RioType)) ;


    //
    // If we don't have a parse tree or
    // we have a goto label or
    // we have a comment line
    // then don't execute anything and return.
    //
    if (!pnodeCmdTree ||
        GotoFlag ||
        pnodeCmdTree->type == REMTYP) {

        return(SUCCESS) ;
    }

    DEBUG((MNGRP, DPLVL, "DISP: type = 0x%02x", pnodeCmdTree->type)) ;

    //
    // Copy node ptr pnodeCmdTree to new node ptr pcmdnode
    // If command is to be detached or pipelined (but not a pipe)
    //  If command is Batch file or Internal or Multi-statement command
    //      "Unparse" tree into string approximating original commandline
    //      Build new command node (pcmdnode) to spawn a child Command.com
    //      Make the string ("/C" prepended) the argument of the new node
    //  Perform redirection on node c
    //  If node pcmdnode is to be detatched
    //      Exec async/discard
    //  else
    //      Exec async/keep but don't wait for retcode (pipelined)
    //  else
    //  If this is a CMDTYP, PARTYP or SILTYP node and there is explicit redirection
    //
    //      Perform redirection on this node
    //  If operator node or a special type (FOR, IF, DET or REM)
    //      Call routine identified by GetFuncPtr() to execute it
    //  Else call FindFixAndRun() to execute the CMDTYP node.
    //  If redirection was performed
    //     Reset redirection
    //

    pcmdnode = (struct cmdnode *)pnodeCmdTree ;

    //
    // If we are called from ePipe and PIPE command then we need
    // to rebuild the command in ascii form (UnParse) and fork
    // off another cmd.exe to execute it.
    //
    if ((RioType == RIO_PIPE && pcmdnode->type != PIPTYP)) {

        //
        // pbCmdbuf is used as tmp in FindCmd and SFE
        //
        // BUGBUG: Why add 3 to MAXTOKLEN
        //
        if (!(pbCmdBuf=mkstr((MAXTOKLEN+3)*sizeof(TCHAR)))) {
            return(DISPERROR) ;
        }

        //
        // If current node to execute is not a command or
        // could not find it as an internal command or
        // it was found as a batch file then
        //    Do the unparse
        //
        if (pcmdnode->type != CMDTYP ||
            FindCmd(CMDHIGH, pcmdnode->cmdline, pbCmdBuf) != -1 ||
            SearchForExecutable(pcmdnode, pbCmdBuf) == SFE_ISBAT) {

            DEBUG((MNGRP, DPLVL, "DISP: Now UnParsing")) ;

            //
            // if pcmdnode an intrnl cmd then pbCmdBuf holds it's switches
            // if pcmdnode was a batch file then pbCmdBuf holds location
            //
            if (UnParse((struct node *)pcmdnode, pbCmdBuf)) {
                return(DISPERROR) ;
            }

            DEBUG((MNGRP, DPLVL, "DISP: UnParsed cmd = %ws", pbCmdBuf)) ;

            //
            // Build a command node with unparsed command
            // Will be exec'd later after redirection is applied
            //
            pcmdnode = (struct cmdnode *)mknode() ;

            if (pcmdnode == NULL)  {
                return(DISPERROR) ;
            }

            pcmdnode->type = CMDTYP ;
            pcmdnode->cmdline = GetEnvVar(ComSpecStr) ;
            pcmdnode->argptr = pbCmdBuf ;
        };

        //
        // Setup I/O redirection
        //
        if (SetRedir((struct node *)pcmdnode, RioType)) {
            return(DISPERROR) ;
        }

        DEBUG((MNGRP, DPLVL, "DISP:Calling ECWork on piped cmd")) ;

        pbCmdBuf[1] = SwitChar ;
        pbCmdBuf[2] = TEXT('S') ;

        comretcode = ECWork(pcmdnode, AI_KEEP, CW_W_NO) ;

        DEBUG((MNGRP, DPLVL, "DISP: ECWork returned %d", comretcode)) ;

    } else {

        //
        // We are here if command was not PIPE
        //
        // If it was a command node or a paren or a silent operator and
        // we have redirection then set redirection.
        //
        if ((pnodeCmdTree->type == CMDTYP ||
             pnodeCmdTree->type == PARTYP ||
             pnodeCmdTree->type == SILTYP ||
             pnodeCmdTree->type == HELPTYP) &&
            pnodeCmdTree->rio) {

            //
            // Set redirection on node.
            //
            if (SetRedir(pnodeCmdTree, RioType)) {
                return(DISPERROR) ;
            }
        }

        //
        // If it is an internal command then find it and execute
        // otherwise locate file load and execute
        //
        if (pnodeCmdTree->type != CMDTYP) {
            comretcode = (*GetFuncPtr(pnodeCmdTree->type))((struct cmdnode *)pnodeCmdTree) ;
        } else {
            comretcode = FindFixAndRun((struct cmdnode *)pnodeCmdTree) ;
        }
    }  // else

    //
    // Reset and redirection that was previously setup
    // pcmdnode is always current node.
    //
    if ((rioCur) && (rioCur->rnod == (struct node *)pcmdnode)) {
        ResetRedir() ;
    }

    DEBUG((MNGRP, DPLVL, "DISP: returning %d", comretcode)) ;
    return(comretcode) ;
}

int
SetRedir(
    IN struct node *pnodeCmdTree,
    IN int RioType
    )

/*++

Routine Description:

    Perform the redirection required by the current node

    Only individual commands and parenthesised statement groups can have
    explicit I/O redirection.  All nodes, however, can tolerate redirection of an
    implicit nature.

Arguments:

    pNode - pointer node containing redirection information
    RioType - indicator of source of redirection request

Return Value:

    SUCCESS if the redirection was successfully set up.
    FAILURE if the redirection was NOT successfully set up.


--*/
{

    struct rio *prio ;
    int i;

    CRTHANDLE OpenStatus;

    BOOLEAN fInputRedirected = FALSE;

    //
    // Temps. Used to hold all of the relocation information for a
    // command.
    //
    struct relem *prelemT ;
    struct relem *prelemT2;

    TCHAR rgchFileName[MAX_PATH];




    DEBUG((MNGRP, RIOLVL, "SETRD:RioType = %d.",RioType)) ;

    prelemT = pnodeCmdTree->rio ;

    //
    // Loop through redirections removing ":" from device names
    // and determining if input has been redirected
    //
    while (prelemT) {

        mystrcpy(prelemT->fname, stripit(prelemT->fname) );

        //
        // skip any redirection that already has been done
        //
        if (prelemT->svhndl) {
            prelemT = prelemT->nxt ;
            continue ;
        }

        //
        // check for and remove any COLON that might be in a device name
        //
        if ((i = mystrlen(prelemT->fname)-1) > 1 && *(prelemT->fname+i) == COLON)
            *(prelemT->fname+i) = NULLC ;

        //
        // If input redirection specified then set flag for later use
        //
        if (prelemT->rdhndl == STDIN) {
            fInputRedirected = TRUE ;
        }

        prelemT = prelemT->nxt ;
    }

    DEBUG((MNGRP, RIOLVL, "SETRD: fInputRedirected = %d",fInputRedirected)) ;

    //
    // Allocate, activate and initialize the rio list element.
    // We must skip this if called from AddRedir (test for RIO_REPROCESS)
    //
    if (RioType != RIO_REPROCESS) {

        if (!(prio=(struct rio *)mkstr(sizeof(struct rio)))) {
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return ( FAILURE ) ;
        } ;

        prio->back = rioCur ;
        rioCur = prio ;
        prio->rnod = pnodeCmdTree ;
        prio->type = RioType ;

        DEBUG((MNGRP, RIOLVL, "SETRD: rio element built.")) ;

    } else {

        prio = rioCur;
    }

    //
    // Once the list has been set up for standard and special cases
    // the actual handle redirection is performed.
    //
    // loop thru the list performing all redirection and error recovery.
    //
    prelemT = pnodeCmdTree->rio ;
    while (prelemT) {
        //
        // Skip any already done.
        //
        if (prelemT->svhndl) {
            prelemT = prelemT->nxt ;
            continue ;
        } ;

        DEBUG((MNGRP, RIOLVL, "SETRD: Old osf handle = %x", CRTTONT(prelemT->rdhndl))) ;

        //
        // Make sure read handle is open and valid before saving it.
        //
        if (FileIsDevice(prelemT->rdhndl) || FileIsPipe(prelemT->rdhndl) ||
            SetFilePointer(CRTTONT(prelemT->rdhndl), 0L, NULL, FILE_CURRENT) != -1) {
                DEBUG((MNGRP, RIOLVL, "SETRD: duping %d", prelemT->rdhndl)) ;
                if ((prelemT->svhndl = Cdup(prelemT->rdhndl)) == BADHANDLE) {

                    DEBUG((MNGRP, RIOLVL, "SETRD: Cdup error=%d, errno=%d", GetLastError(), errno)) ;
                    PutStdErr(MSG_RDR_HNDL_CREATE, ONEARG, argstr1(TEXT("%d"), (unsigned long)prelemT->rdhndl)) ;
                    prelemT->svhndl = 0 ;
                    ResetRedir() ;
                    return(FAILURE) ;
                }

                DEBUG((MNGRP, RIOLVL, "SETRD: closing %d", prelemT->rdhndl)) ;
                Cclose(prelemT->rdhndl) ;

                DEBUG((MNGRP,RIOLVL,"SETRD: save handle = %d", prelemT->svhndl));
                DEBUG((MNGRP,RIOLVL,"SETRD: --->osf handle = %x", CRTTONT(prelemT->svhndl))) ;

        } else {

            DEBUG((MNGRP, RIOLVL, "SETRD: FileIsOpen ret'd FALSE")) ;
            PutStdErr(MSG_RDR_HNDL_OPEN, ONEARG, argstr1(TEXT("%d"), (unsigned long)prelemT->rdhndl)) ;
            prelemT->svhndl = 0 ;
            ResetRedir() ;
            return(FAILURE) ;

        }


        //
        // Is file name the command seperator character '&'
        //
        if (*prelemT->fname == CSOP) {

            DEBUG((MNGRP,RIOLVL,"SETRD: Handle substitution, %ws %d", prelemT->fname, prelemT->rdhndl)) ;

            *(prelemT->fname+2) = NULLC ;
            if (Cdup2(*(prelemT->fname+1) - TEXT('0'), prelemT->rdhndl) == BADHANDLE) {
                DEBUG((MNGRP, RIOLVL, "SETRD: Cdup2 error=%d, errno=%d", GetLastError(), errno)) ;
                ResetRedir() ;

                PutStdErr(MSG_RDR_HNDL_CREATE, ONEARG, argstr1(TEXT("%d"), (ULONG)prelemT->rdhndl)) ;
                return(FAILURE) ;
            } ;

            DEBUG((MNGRP,RIOLVL,"SETRD: %c forced to %d",*(prelemT->fname+1), (ULONG)prelemT->rdhndl)) ;

        } else {

            //
            // redirecting input from a file. Check to see if file
            // exists and can be opened for input.
            //
            if (prelemT->rdop == INOP) {

                DEBUG((MNGRP,RIOLVL,"SETRD: File in = %ws",prelemT->fname)) ;

                //
                // Try to open file localy first
                //
                if ((OpenStatus = Copen(prelemT->fname, O_RDONLY|O_BINARY)) == BADHANDLE) {

                    //
                    // Now try the DPATH (data path)
                    //
                    if ( SearchPath( AppendStr,
                                    prelemT->fname,
                                    NULL,
                                    MAX_PATH,
                                    rgchFileName,
                                    NULL ) != 0 ) {
                            OpenStatus = Copen(rgchFileName, O_RDONLY|O_BINARY) ;
                    }
                }

            } else {

                //
                // We are not redirecting input so must be output
                //

                DEBUG((MNGRP,RIOLVL,"SETRD: File out = %ws",prelemT->fname)) ;

                //
                // Make sure sure we can open the file for output
                //
                OpenStatus = Copen(prelemT->fname, prelemT->flag ? OP_APPEN : OP_TRUNC) ;
            }

            //
            // If the handle to be redirected was not the lowest numbered,
            // unopened handle when open was called, the current handle must
            // be forced to it, the handle returned by open must be closed.
            //
            // BUGBUG: why must it be force to lowest numbered
            //
            if (OpenStatus != BADHANDLE && OpenStatus != prelemT->rdhndl) {

                DEBUG((MNGRP,RIOLVL,"SETRD: Handles don't match...")) ;
                DEBUG((MNGRP,RIOLVL,"SETRD: ...forcing %d to %d", i, (ULONG)prelemT->rdhndl)) ;

                if (Cdup2(OpenStatus, prelemT->rdhndl) == BADHANDLE) {

                    DEBUG((MNGRP, RIOLVL, "SETRD: Cdup2 error=%d, errno=%d", GetLastError(), errno)) ;
                    Cclose(OpenStatus) ;
                    ResetRedir() ;

                    PutStdErr(MSG_RDR_HNDL_CREATE, ONEARG, argstr1(TEXT("%d"), (ULONG)prelemT->rdhndl)) ;
                    return(FAILURE) ;

                } else {

                    Cclose(OpenStatus) ;
                    OpenStatus = prelemT->rdhndl ;
                }
            }

            //
            // Copen error processing must be delayed to here to allow the
            // above Cdup2 to occur if necessary.  Otherwise, the call to
            // ResetRedir in the error handler would attempt to close the
            // wrong handle and leave a bogus handle open.
            //
            if (OpenStatus == BADHANDLE) {

                DEBUG((MNGRP,RIOLVL,"SETRD: Bad Open, DosErr = %d",DosErr)) ;
                ResetRedir() ;

                PrtErr(DosErr) ;
                return(FAILURE) ;
            }

            DEBUG((MNGRP, RIOLVL, "SETRD: new handle = %d", OpenStatus)) ;
            DEBUG((MNGRP,RIOLVL,"SETRD: --->osf handle = %x", CRTTONT(OpenStatus))) ;

            //
            // Keep highest numbered handle
            //
            prio->stdio = OpenStatus ;

        } // else

        prelemT = prelemT->nxt ;

    } // while


    return(SUCCESS) ;
}


AddRedir(
    IN struct cmdnode *pcmdnodeOriginal,
    IN struct cmdnode *pcmdnodeNew
    )
/*++

Routine Description:

    Add redirection from a new node to an existing one.  Walk the
    redirection list of the old node for each element in the new.
    Duplicates are removed from the old and replaced by the new,
    while unique new ones are added to the end.  When the two lists
    are merged, reprocess the redirection.

Arguments:

    pcmdnodeOriginal - original node to be added to
    pcmdnodeNew      - new node to merge.

Return Value:

    SUCCESS if the redirection was successfully merged.
    FAILURE otherwise.

--*/

{

    struct relem *prelemOriginal ;
    struct relem *prelemNew ;
    struct relem *prelemEnd ;           // Ptr to end of original list

    //
    // Flag to set Stack Minimum
    //
    BOOLEAN fSetStackMin = FALSE;

    PTCHAR oldname ;            /* Sanity check                    */
    struct rio *rn ;    /* Possible rio element    */

    //
    // Won't be here unless pcmdnodeNew-reio exists
    //
    prelemNew = pcmdnodeNew->rio ;

    // If there was no redirection associated with the original node, we must
    // also create a rio element so that the redirection can be reset at
    // command completion or receipt of signal.  We have to create it here
    // rather than in SetRedir in order to include it on the data stack when
    // we set a new level.

    if (!(prelemEnd = prelemOriginal = pcmdnodeOriginal->rio)) {

        DEBUG((MNGRP, RIOLVL, "ADDRD: No old redirection.")) ;

        //
        // New list becomes original
        //
        pcmdnodeOriginal->rio = prelemNew ;

        if (!(rn=(struct rio *)mkstr(sizeof(struct rio)))) {
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return(FAILURE) ;
        }

        //
        // Create dummy redirection node.
        //
        rn->back = rioCur ;
        rioCur = rn ;
        rn->rnod = (struct node *)pcmdnodeOriginal ;
        rn->type = RIO_BATLOOP ;

        DEBUG((MNGRP, RIOLVL, "ADDRD: rio element built.")) ;

        fSetStackMin = TRUE ;       /* Must save current datacount     */
        prelemNew = NULL ;            /* Skip the while loops            */
    } else {

        //
        // Find the end of the orignal list
        //
        while (prelemEnd->nxt) {
            prelemEnd = prelemEnd->nxt ;
        }
    }

    //
    // If prelemNew is non-null, we've two lists which we integrate by
    // eliminating any duplicate entries and adding any unique entries in
    // the new list to the end of the original.  Note that if unique entries
    // exist, we must save the current data count to avoid losing their
    // malloc'd data when we go on to SetBat().
    //

    //
    // For each new redirection, look at the original
    //
    while (prelemNew) {

        while(prelemOriginal) {

            //
            // Do we have a duplicate
            //
            if (prelemNew->rdhndl != prelemOriginal->rdhndl) {
                prelemOriginal = prelemOriginal->nxt ;
                continue ;
            } else {

                if (prelemOriginal->svhndl && (prelemOriginal->svhndl != BADHANDLE)) {
                    //
                    // BUGBUG put an assert here
                    //
                    Cdup2(prelemOriginal->svhndl, prelemOriginal->rdhndl) ;
                    Cclose(prelemOriginal->svhndl) ;
                } else {
                    if (prelemOriginal->svhndl == BADHANDLE) {
                        Cclose(prelemOriginal->rdhndl) ;
                    }
                }
                prelemOriginal->svhndl = 0 ; /* ...and replace it     */
                prelemOriginal->flag = prelemNew->flag ;
                prelemOriginal->rdop = prelemNew->rdop ;
                oldname = prelemOriginal->fname ;
                prelemOriginal->fname = resize(prelemOriginal->fname, mystrlen(prelemNew->fname) + 1) ;
                mystrcpy(prelemOriginal->fname, prelemNew->fname) ;
                if (prelemOriginal->fname != oldname) {
                    fSetStackMin = TRUE ;
                }
                pcmdnodeNew->rio = prelemNew->nxt ;
                break ;
            }
        }

        //
        // If no old entry remove from new and add to original
        // update the end pointer, zero next pointer and preserve datacount
        //
        if (prelemNew == pcmdnodeNew->rio) {
            pcmdnodeNew->rio = prelemNew->nxt ;
            prelemEnd->nxt = prelemNew ;
            prelemEnd = prelemEnd->nxt ;
            prelemEnd->nxt = NULL ;
            fSetStackMin = TRUE ;
        }
        prelemNew = pcmdnodeNew->rio ;
        prelemOriginal = pcmdnodeOriginal->rio ;
    }

    //
    // All duplicates are eliminated.   Now save the data count and call
    // SetRedir to reprocess the redirection list for any unimplimented
    // redirection (io->svhndl == 0).
    //

    if (fSetStackMin) {
        if (CurBat->stacksize < (CurBat->stackmin = DCount)) {
            CurBat->stacksize = DCount ;
        }
    }
    return(SetRedir((struct node *)pcmdnodeOriginal, RIO_REPROCESS)) ;
}
void
ResetRedir()

/*++

Routine Description:

    Reset the redirection identified by the last rio list element
    as pointed to by rioCur.  When finished, remove the rio element
    from the list.

Arguments:


Return Value:


--*/

{
    struct rio *prio = rioCur;
    struct relem *prelemT ;
    CRTHANDLE handleT;

    DEBUG((MNGRP, RIOLVL, "RESETR: Entered.")) ;

    prelemT = prio->rnod->rio ;

    while (prelemT) {

        if (prelemT->svhndl && (prelemT->svhndl != BADHANDLE)) {

            DEBUG((MNGRP,RIOLVL,"RESETR: Resetting %d",(ULONG)prelemT->rdhndl)) ;
            DEBUG((MNGRP,RIOLVL,"RESETR: From save %d",(ULONG)prelemT->svhndl)) ;

            handleT = Cdup2(prelemT->svhndl, prelemT->rdhndl) ;
            Cclose(prelemT->svhndl) ;

            DEBUG((MNGRP,RIOLVL,"RESETR: Dup2 retcode = %d", handleT)) ;

        } else {

            if (prelemT->svhndl == BADHANDLE) {

                DEBUG((MNGRP,RIOLVL,"RESETR: Closing %d",(ULONG)prelemT->rdhndl)) ;

                Cclose(prelemT->rdhndl) ;
            }
        }

        prelemT->svhndl = 0 ;
        prelemT = prelemT->nxt ;
    }

    //
    // Kill list element
    //
    rioCur = prio->back ;

    DEBUG((MNGRP, RIOLVL, "RESETR: List element destroyed.")) ;
}



int
FindFixAndRun (
    IN struct cmdnode *pcmdnode
    )
/*++

Routine Description:

    If the command name is in the form d: or, just change drives.
    Otherwise, search for the nodes command name in the jump table.
    If it is found, check the arguments for bad drivespecs or unneeded
    switches and call the function which executes the command.
    Otherwise, assume it is an external command and call ExtCom.


Arguments:

    pcmdnode - the node of the command to be executed


Return Value:

    SUCCESS or FAILURE if changing drives.
    Otherwise, whatever is returned by the function which is called to
    execute the command.

--*/

{
    PTCHAR pszTokStr ;

    USHORT DriveNum;
    ULONG  JmpTblIdx;
    TCHAR  cname[MAX_PATH] ;
    TCHAR   cflags;
    int    (*funcptr)(struct cmdnode *) ;
    unsigned cbTokStr;
    PTCHAR   pszTitle;
    ULONG   rc;


    //
    // I haven't found where in CMD we end up with NULL pointer here
    // (all failing mallocs cause CMD to exit)
    // however I saw one strange stress failure.
    // So lets not cause AV and just return FAILURE if NULL.
    //

    if (pcmdnode->cmdline == NULL)
        return(FAILURE) ;


    //
    // Validate any drive letter
    //
    if (*(pcmdnode->cmdline+1) == COLON) {
        if (!IsValidDrv(*pcmdnode->cmdline)) {

            PutStdErr(ERROR_INVALID_DRIVE, NOARGS);
            return(FAILURE) ;

        } else {

            //
            // Make sure it isn't locked either
            //
            if ( IsDriveLocked(*pcmdnode->cmdline)) {
                PutStdErr( GetLastError() , NOARGS);
                return(FAILURE) ;
            }
        }

        //
        // Pull out drive letter and convert to drive number
        // BUGBUG: it is   not neccessary to do this conversion
        //
        DriveNum = (USHORT)(_totupper(*pcmdnode->cmdline) - SILOP) ;

        //
        // If this is just a change in drive do it here
        //
        if (mystrlen(pcmdnode->cmdline) == 2) {

            //
            // ChangeDrive set CurDrvDir in addition to changing the drive
            ChangeDrive(DriveNum) ;
            DEBUG((MNGRP,DPLVL,"FFAR: Drv chng to %ws", CurDrvDir)) ;
            return(SUCCESS) ;
        }

        //
        // Note that if the cmdline contains a drivespec, no attempt is made at
        // internal command matching whatsoever.
        //
        return(ExtCom(pcmdnode)) ;
    }

    //
    // The sequence below works as follows:
    // - A match between the previously-parsed first non-delimiter character
    //   group in the cmdline and the command table is attempted.  A match
    //   sets JmpTblIdx to the command index; no match sets JmpTblIdx to -1.
    // - FixCom is then called, and using the value of 'i', it detects cases
    //   of internal commands only (i == -1) which have no standard delimiter
    //   (whitespace or "=;,") between them and their arguments such as the
    //   "cd\foo". Note that a file foo.exe in subdirectory "cd" cannot be
    //   executed except through full path or drive specification. FixCom
    //   actually fixes up the cmdline and argptr fields of the node.
    // - The command is then executed using ExtCom (i == -1) or the internal
    //   function indicated by the index
    //
    // Added second clause to detect REM commands which were parsed incorrectly
    // as CMDTYP due to semi-delimiter characters appended.  If REM, we know
    // its OK, so just return success.  If any other of the special types,
    // FOR, DET, EXT, etc., allow to continue and fail in ExtCom since they
    // weren'tparsed correctly and will bomb.
    //
    JmpTblIdx = FindAndFix( pcmdnode, (PTCHAR )&cflags ) ;

    DEBUG((MNGRP, DPLVL, "FFAR: After FixCom pcmdnode->cmdline = '%ws'", pcmdnode->cmdline)) ;

    //
    // Check if it was not an internal command, if so then exec it
    //
    if (JmpTblIdx == -1) {

        DEBUG((MNGRP, DPLVL, "FFAR: Calling ExtCom on %ws", pcmdnode->cmdline)) ;
        return(ExtCom(pcmdnode)) ;

    }

    //
    // CMD was found in table.  If function field is NULL as in the
    // case of REM, this is a dummy entry and must return SUCCESS.
    //
    if ((funcptr = GetFuncPtr(JmpTblIdx)) == NULL) {

        DEBUG((MNGRP, DPLVL, "FFAR: Found internal with NULL entry")) ;
        DEBUG((MNGRP, DPLVL, "      Returning SUCESS")) ;
        return(SUCCESS) ;

    }

    //
    // If the command is supposed to have the drivespecs on its args
    // validated before the command is executed, do it. If the command
    // is not allowed toto contain switches and it has one, complain.
    //

    //
    // Set up extra delimiter for seperating out switches
    //
    cname[0] = SwitChar;
    cname[1] = NULLC;

    pszTokStr = TokStr(pcmdnode->argptr, cname, TS_SDTOKENS) ;

    // this hack to allow environment variables to contain /?
    if (JmpTblIdx != SETTYP || !pszTokStr || (_tcsncmp(pszTokStr,TEXT("/\0?"),4) == 0)) {
        // this is to exclude START command
        if (JmpTblIdx != STRTTYP) {
            if (CheckHelpSwitch(JmpTblIdx, pszTokStr) ) {
                return( FAILURE );
            }
        }
    }
    DEBUG((MNGRP, DPLVL, "FFAR: Internal command, about to validate args")) ;
    for (;(pszTokStr != NULL) && *pszTokStr ; pszTokStr += mystrlen(pszTokStr)+1) {

        cbTokStr = mystrlen(pszTokStr);
        mystrcpy( pszTokStr, stripit( pszTokStr ) );

        DEBUG((MNGRP, DPLVL, "FFAR: Checking args; arg = %ws", pszTokStr)) ;

        if ((cflags & CHECKDRIVES) && *(pszTokStr+1) == COLON) {

            if (!IsValidDrv(*pszTokStr)) {

                PutStdErr(ERROR_INVALID_DRIVE, NOARGS);
                return(LastRetCode = FAILURE) ;

            } else {

                //
                // If not the copy command (A->B B->A swaps)
                // then check if drive is locked
                // if drive locked then
                // display error return code message
                // terminate this command's processing
                //
                if (JmpTblIdx != CPYTYP) {

                    if ( IsDriveLocked(*pszTokStr)) {

                        PutStdErr( GetLastError() , NOARGS);
                        return(LastRetCode = FAILURE) ;
                    }
                }
            }
        }

        if ((cflags & NOSWITCHES) && (pszTokStr != NULL) && *pszTokStr == SwitChar) {

            PutStdErr(MSG_BAD_SYNTAX, NOARGS);
            return(LastRetCode = FAILURE) ;
        }

    }

    DEBUG((MNGRP, DPLVL, "FFAR: calling function, cmd = `%ws'", pcmdnode->cmdline)) ;
    //
    // Call internal routine to execute the command
    //
    if ((pszTitle = GetTitle(pcmdnode)) != NULL) {
        SetConTitle(pszTitle);
    }

    rc = (*funcptr)(pcmdnode);

    ResetConTitle(pszTitleCur);

    return(rc) ;
}

int
FindAndFix (
    IN struct cmdnode *pcmdnode,
    IN PTCHAR pbCmdFlags
    )

/*++

Routine Description:

    This routine separates the command and its following
    switch character if there is no space between the
    command and the switch character.

    This routine is used for both left side and right side
    of PIPE.

Arguments:

    pcmdnode - pointer to node the contains command to locate
    pbCmdFlags -

Return Value:


--*/

{
    TCHAR  chCur;           // current character we are looking at
    TCHAR  rgchCmdStr[MAX_PATH] ;
    PTCHAR pszArgT;         // Temp. used to build a new arguemt string

    ULONG JmpTableIdx;      // index into jump table of function pointers
    ULONG iCmdStr;          // index into  command string
    ULONG cbCmdStr;         // length of command string

    BOOLEAN fQuoteFound, fQuoteFound2;
    BOOLEAN fDone;


    fQuoteFound =  FALSE;
    fQuoteFound2 = FALSE;

    //
    // Extract only commnand from the command string (pcmdnode->cmdline)
    //
    for (iCmdStr = 0 ; iCmdStr < MAX_PATH-1; iCmdStr++) {

        chCur = *(pcmdnode->cmdline + iCmdStr);

        //
        // If we found a quote invert the current quote state
        // for both first quote (fQuoteFound) and end quote (fQuoteFound2)
        //
        if ( chCur == QUOTE ) {

            fQuoteFound = (BOOLEAN)!fQuoteFound;
            fQuoteFound2 = (BOOLEAN)!fQuoteFound;
        }

        //
        // If we have a character and
        // have found either a begin or end quote or cur char is not delimeter
        // and cur char is not a special (+[] etc.) delimiter
        //
        if ((chCur) &&
            ( fQuoteFound || fQuoteFound2 || !mystrchr(Delimiters,chCur) &&
            !mystrchr(Delim2,chCur))) {

            rgchCmdStr[iCmdStr] = chCur ;
            fQuoteFound2 = FALSE;

        }
        else {

            break ;
        }
    }
    if (iCmdStr == 0) {
        return -1;
    }

    rgchCmdStr[iCmdStr] = NULLC ;


    //
    // See if command is in jump table (is an internal command)
    // If it is not found amoung the normal internal command
    // check amoung the special parse type if it was a comment
    //
    if ((JmpTableIdx = FindCmd(CMDHIGH, rgchCmdStr, pbCmdFlags)) == -1) {
        if (FindCmd(CMDMAX, rgchCmdStr, pbCmdFlags) == REMTYP) {
                    return(REMTYP) ;
        }
    } else if (JmpTableIdx == GOTYP)
        pcmdnode->flag = CMDNODE_FLAG_GOTO;

    fQuoteFound = FALSE;
    fQuoteFound2 = FALSE;

    //
    // If the command is not found, check the length of command string
    // for the case of DBCS. Count the characters that are not white space
    // remaining in command
    if ( JmpTableIdx == -1 ) {

        fDone = FALSE;
        while ( !fDone ) {
            chCur = *(pcmdnode->cmdline+iCmdStr);
            if ( chCur && chCur == QUOTE ) {
                fQuoteFound = (BOOLEAN)!fQuoteFound;
                fQuoteFound2 = (BOOLEAN)!fQuoteFound;
            }
            if ( chCur && ( fQuoteFound || fQuoteFound2 ||
                !_istspace(chCur) &&
                !mystrchr(Delimiters, chCur) &&
                !(chCur == SwitChar))) {

                iCmdStr++;
                fQuoteFound2 = FALSE;
            } else {
                fDone = TRUE;
            }
        }
    }

    //
    // If cmdstr contains more than command, strip of extra part
    // and put it in front of the existing command argument pcmdnode-argptr
    //
    //
    if (iCmdStr != (cbCmdStr = mystrlen(pcmdnode->cmdline))) {
        int ArgLen;

        ArgLen = mystrlen(pcmdnode->argptr);
        ArgLen += cbCmdStr;

        if (!(pszArgT = mkstr(ArgLen*sizeof(TCHAR)))) {

            PutStdErr(MSG_NO_MEMORY, NOARGS);
            Abort() ;
        }
        //
        // create argument string and copy the 'extra' part of command
        // it.
        //
        mystrcpy(pszArgT, pcmdnode->cmdline+iCmdStr) ;
        //
        // If we have a argument pointer stuff in the front
        //
        if (pcmdnode->argptr) {

            mystrcat(pszArgT, pcmdnode->argptr) ;

        }
        pcmdnode->argptr = pszArgT ;
        *(pcmdnode->cmdline+iCmdStr) = NULLC ;
    }

    return(JmpTableIdx) ;
}





int
UnParse(
    IN struct node *pnode,
    IN PTCHAR pbCmdBuf )

/*++

Routine Description:

    Do setup and call UnBuild to deparse a node tree.

Arguments:

    pnode - pointer to root of parse tree to UnParse
    pbCmdBuf -
    Uses Global pointer CBuf and assumes a string of MAXTOKLEN+1 bytes
    has already been allocated to it (as done by Dispatch).

Return Value:


--*/

{

    int rc;

    DEBUG((MNGRP, DPLVL, "UNPRS: Entered")) ;

    if (!pnode) {

        DEBUG((MNGRP, DPLVL, "UNPRS: Found NULL node")) ;
        return(FAILURE) ;
    }

    //
    // Leave space in front of command for a /s
    // Setup command buffer for a single command execution
    //
    pbCmdBuf[0] = SPACE ;
    pbCmdBuf[1] = SPACE ;
    pbCmdBuf[2] = SPACE ;
    pbCmdBuf[3] = SPACE ;
    pbCmdBuf[4] = SwitChar ;
    pbCmdBuf[5] = TEXT('C') ;
    pbCmdBuf[6] = TEXT('\"');
    pbCmdBuf[7] = NULLC;

    //
    // Setup to handle an exception during detach.
    if (setjmp(CmdJBuf2)) {
        DEBUG((MNGRP, DPLVL, "UNPRS: Longjmp return occurred!")) ;
        return(FAILURE) ;
    }

    //
    // DisAssemble the current command
    //
    rc = (UnBuild(pnode, pbCmdBuf)) ;
    mystrcat( pbCmdBuf, TEXT("\"") );
    return( rc );
}

UnBuild(
    IN struct node *pnode,
    IN PTCHAR pbCmdBuf
    )

/*++

Routine Description:

    Recursively take apart a parse tree of nodes, building a string of
    their components.

Arguments:

    pnode - root of parse tree to UnBuild
    pbCmdBuf - Where to put UnBuilt command

Return Value:


--*/

{

    //
    // Different kinds of nodes to Unbuild
    //
    struct cmdnode *pcmdnode;
    struct fornode *pfornode ;
    struct ifnode *pifnode ;
    PTCHAR op ;

    DEBUG((MNGRP, DPLVL, "UNBLD: Entered")) ;

    switch (pnode->type) {

    case CSTYP:
    case ORTYP:
    case ANDTYP:
    case PIPTYP:
    case PARTYP:
    case SILTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found OPERATOR")) ;

        UnDuRd(pnode, pbCmdBuf) ;

        switch (pnode->type) {

        case CSTYP:

            op = CSSTR ;
            break ;

        case ORTYP:

            op = ORSTR ;
            break ;

        case ANDTYP:

            op = ANDSTR ;
            break ;

        case PIPTYP:

            op = PIPSTR ;
            break ;

        case PARTYP:

            SPutC(pbCmdBuf, LEFTPSTR,YSPC) ;
            op = RPSTR ;
            break ;

        case SILTYP:

            SPutC(pbCmdBuf, SILSTR,YSPC) ;
            op = SPCSTR ;
            break ;
        } ;

        //
        // Recurse down undoing the left hand side
        //
        UnBuild(pnode->lhs, pbCmdBuf) ;

        //
        // Now that left side there copy in operator and do right side
        //
        SPutC(pbCmdBuf, op,YSPC) ;
        if (pnode->type != PARTYP && pnode->type != SILTYP)
                UnBuild(pnode->rhs, pbCmdBuf) ;
        break ;

    case FORTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found FORTYP")) ;
        pfornode = (struct fornode *) pnode ;

        //
        // Put in the FOR keywoard, arguements and list
        //
        SPutC( pbCmdBuf, pfornode->cmdline,YSPC) ;
        SPutC( pbCmdBuf, LEFTPSTR,YSPC) ;
        SPutC( pbCmdBuf, pfornode->arglist,NSPC) ;
        SPutC( pbCmdBuf, RPSTR,NSPC) ;
        SPutC( pbCmdBuf, pfornode->cmdline+DOPOS,YSPC) ;

        //
        // Now get the for body
        //
        UnBuild(pfornode->body, pbCmdBuf) ;
        break ;

    case IFTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found IFTYP")) ;

        //
        // put ine IF keyword
        pifnode = (struct ifnode *) pnode ;
        SPutC( pbCmdBuf, pifnode->cmdline,YSPC) ;

        //
        // Get the condition part of the statement
        //
        UnBuild((struct node *)pifnode->cond, pbCmdBuf) ;

        //
        // Unbuild the body of the IF
        //
        UnBuild(pifnode->ifbody, pbCmdBuf) ;
        if (pifnode->elsebody) {
                SPutC( pbCmdBuf, pifnode->elseline,YSPC) ;
                UnBuild(pifnode->elsebody, pbCmdBuf) ;
        } ;
        break ;

    case NOTTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found NOTTYP")) ;
        pcmdnode = (struct cmdnode *) pnode ;
        SPutC( pbCmdBuf, pcmdnode->cmdline,YSPC) ;
        UnBuild((struct node *)pcmdnode->argptr, pbCmdBuf) ;
        break ;

    case REMTYP:
    case CMDTYP:
    case ERRTYP:
    case EXSTYP:
    case STRTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found CMDTYP")) ;
        pcmdnode = (struct cmdnode *) pnode ;
        SPutC( pbCmdBuf, pcmdnode->cmdline,YSPC) ;
        if (pcmdnode->argptr)
                SPutC( pbCmdBuf, pcmdnode->argptr,NSPC) ;
        UnDuRd((struct node *)pcmdnode, pbCmdBuf) ;
        break ;

    case HELPTYP:
        DEBUG((MNGRP, DPLVL, "UNBLD: Found HELPTYP")) ;
        if (LastMsgNo == MSG_HELP_FOR) {
            SPutC( pbCmdBuf, TEXT("FOR /?"), YSPC) ;
        }
        else if (LastMsgNo == MSG_HELP_IF) {
            SPutC( pbCmdBuf, TEXT("IF /?"), YSPC) ;
        }
        else if (LastMsgNo == MSG_HELP_REM) {
            SPutC( pbCmdBuf, TEXT("REM /?"), YSPC) ;
        }
        else {
            DEBUG((MNGRP, DPLVL, "UNBLD: Unknown Type!")) ;
            longjmp(CmdJBuf2,-1) ;
        }

        break;

    default:

        DEBUG((MNGRP, DPLVL, "UNBLD: Unknown Type!")) ;
        longjmp(CmdJBuf2,-1) ;
    }

    return(SUCCESS) ;

}

void
UnDuRd(
    IN struct node *pnode,
    IN PTCHAR pbCmdBuf
    )
/*++

Routine Description:

    Unparse any input or output redirection associated with the
    current node.

Arguments:

    pnode - current parse tree node
    pbCmdBuf - buffer holding command

Return Value:


--*/

{

    struct relem *prelem ;
    TCHAR tmpstr[2] ;

    DEBUG((MNGRP, DPLVL, "UNDURD: Entered")) ;

    tmpstr[1] = NULLC ;
    prelem = pnode->rio ;
    while (prelem) {

        //
        // BUGBUG this makes big time assumption about size of
        // handle
        //
        tmpstr[0] = (TCHAR)prelem->rdhndl + (TCHAR)'0' ;

        SPutC( pbCmdBuf, tmpstr,YSPC) ;

        if (prelem->rdop == INOP)
            SPutC( pbCmdBuf, INSTR,NSPC) ;
        else
            SPutC( pbCmdBuf, prelem->flag ? APPSTR : OUTSTR,NSPC) ;

        SPutC( pbCmdBuf, prelem->fname,NSPC) ;
        prelem = prelem->nxt ;
    }
}


void SPutC(
    IN PTCHAR pbCmdBuf,
    IN PTCHAR pszInput,
    IN int flg
    )
/*++

Routine Description:

    If within length limits, add the current substring to the
    command under construction delimiting with a space.

Arguments:

    pbCmdBuf - Where to put string
    pszInputString - String to put in pbCmdBuf
    flg - Flags controling placement of spaces

Return Value:


--*/

{
    DEBUG((MNGRP, DPLVL, "SPutC: Entered, Adding '%ws'",pszInput)) ;

    if ((mystrlen(pbCmdBuf) + mystrlen(pszInput) + 1) > MAXTOKLEN) {

        PutStdErr(MSG_LINES_TOO_LONG, NOARGS);
        longjmp(CmdJBuf2,-1) ;
    }

    if (flg && (*(pbCmdBuf+mystrlen(pbCmdBuf)-1) != SPACE) && (*pszInput != SPACE)) {

        SpaceCat(pbCmdBuf,pbCmdBuf,pszInput) ;

    } else {

        mystrcat(pbCmdBuf,pszInput) ;

    }
}
