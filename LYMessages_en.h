/*
 * Lynx - Hypertext navigation system
 *
 *   (c) Copyright 1992, 1993, 1994 University of Kansas
 *	 1995, 1996: GNU General Public License
 */
#ifndef LYMESSAGES_EN_H
#define LYMESSAGES_EN_H

/*******************************************************************
 * The following definitions are for statusline prompts, messages,
 * or warnings issued by Lynx during program execution.  You can
 * modify them to make them more appropriate for you site, and/or
 * to use other languages.  Links to collections of alternate
 * definitions, developed by the Lynx User Community, are maintained
 * in Lynx links:
 *
 *    http://www.crl.com/~subir/lynx.html
 *
 * You can substitute one of those as LYMessages_en.h, or modify
 * the header inclusion statement in your userdefs.h to include
 * one of those with a different name.
 */
#ifdef QUIT_DEFAULT_YES
#define REALLY_QUIT "Are you sure you want to quit? [Y] "
#ifdef VMS
#define REALLY_EXIT "Really exit from Lynx? [Y] "
#endif /* VMS */
#else
#define REALLY_QUIT "Are you sure you want to quit? [N] "
#ifdef VMS
#define REALLY_EXIT "Really exit from Lynx? [N] "
#endif /* VMS */
#endif /* QUIT_DEFAULT_YES */
#define CANCELLED "Cancelled!!!"
#define CANCELLING "Cancelling!"
#define NO_CANCEL "Excellent!!!"
#define OPERATION_DONE "Done!"
#define BAD_REQUEST "Bad request!"
#define HELP \
 "Commands: Use arrow keys to move, '?' for help, 'q' to quit, '<-' to go back."
#define MOREHELP \
 "-- press space for more, use arrow keys to move, '?' for help, 'q' to quit."
#define MORE "-- press space for next page --"
#define FORM_LINK_TEXT_MESSAGE \
 "(Text entry field) Enter text.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_TEXT_UNM_MSG \
 "UNMODIFIABLE form text field.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_TEXT_SUBMIT_MESSAGE \
 "(Form field) Enter text.  Use <return> to submit ('x' for no cache)."
#define FORM_LINK_TEXT_RESUBMIT_MESSAGE \
 "(Form field) Enter text.  Use <return> to submit, arrows or tab to move off."
#define FORM_LINK_TEXT_SUBMIT_UNM_MSG \
 "UNMODIFIABLE form field.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_TEXT_SUBMIT_MAILTO_MSG \
 "(mailto form field) Enter text.  Use <return> to submit, arrows to move off."
#define FORM_LINK_TEXT_SUBMIT_MAILTO_DIS_MSG \
 "(mailto form field) Mail is disallowed so you cannot submit."
#define FORM_LINK_PASSWORD_MESSAGE \
 "(Password entry field) Enter text.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_PASSWORD_UNM_MSG \
 "UNMODIFIABLE form password.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_CHECKBOX_MESSAGE \
 "(Checkbox Field)   Use right-arrow or <return> to toggle."
#define FORM_LINK_CHECKBOX_UNM_MSG \
 "UNMODIFIABLE form checkbox.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_RADIO_MESSAGE \
 "(Radio Button)   Use right-arrow or <return> to toggle."
#define FORM_LINK_RADIO_UNM_MSG \
 "UNMODIFIABLE form radio button.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_SUBMIT_MESSAGE \
 "(Form submit button) Use right-arrow or <return> to submit ('x' for no cache)."
#define FORM_LINK_RESUBMIT_MESSAGE \
 "(Form submit button) Use right-arrow or <return> to submit."
#define FORM_LINK_SUBMIT_DIS_MSG \
 "DISABLED form submit button.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_SUBMIT_MAILTO_MSG \
 "(mailto form submit button) Use right-arrow or <return> to submit."
#define FORM_LINK_SUBMIT_MAILTO_DIS_MSG \
 "(mailto form submit button) Mail is disallowed so you cannot submit."
#define FORM_LINK_RESET_MESSAGE \
 "(Form reset button)   Use right-arrow or <return> to reset form to defaults."
#define FORM_LINK_RESET_DIS_MSG \
 "DISABLED form reset button.  Use UP or DOWN arrows or tab to move off."
#define FORM_LINK_OPTION_LIST_MESSAGE \
 "(Option list) Hit return and use arrow keys and return to select option."
#define FORM_LINK_OPTION_LIST_UNM_MSG \
 "UNMODIFIABLE option list.  Use return or arrow keys to review or leave."
#define NORMAL_LINK_MESSAGE \
 "(NORMAL LINK)   Use right-arrow or <return> to activate."
#define LINK_NOT_FOUND "The resource requested is not available at this time."
#define WWW_WAIT_MESSAGE "Getting %s"
#define ADVANCED_URL_MESSAGE "URL: %s"
#define WWW_FAIL_MESSAGE "Unable to access WWW file!!!"
#define WWW_INDEX_MESSAGE "This is a searchable index.  Use %s to search."
#define WWW_INDEX_MORE_MESSAGE \
 "--More--  This is a searchable index.  Use %s to search."
#define BAD_LINK_NUM_ENTERED "You have entered an invalid link number."
#define SOURCE_HELP \
 "Currently viewing document source.  Press '\\' to return to rendered version."
#define NOVICE_LINE_ONE \
 "  Arrow keys: Up and Down to move. Right to follow a link; Left to go back.  \n"
#define NOVICE_LINE_TWO \
 " H)elp O)ptions P)rint G)o M)ain screen Q)uit /=search [delete]=history list \n"
#define NOVICE_LINE_TWO_A \
 "  O)ther cmds  H)elp  K)eymap  G)oto  P)rint  M)ain screen  o)ptions  Q)uit  \n"
#define NOVICE_LINE_TWO_B \
 "  O)ther cmds  B)ack  E)dit  D)ownload ^R)eload ^W)ipe screen  search doc: / \n"
#define NOVICE_LINE_TWO_C \
 "  O)ther cmds  C)omment  History: <delete>  Bookmarks: V)iew, A)dd, R)emove  \n"
#define FORM_NOVICELINE_ONE \
 "            Enter text into the field by typing on the keyboard              "
#define FORM_NOVICELINE_TWO \
"    Ctrl-U to delete all text in field, [Backspace] to delete a character    "
#define SUBMITTING_FORM "Submitting form..."
#define RESETTING_FORM "Resetting form..."
#define RELOADING_FORM \
 "Reloading document.  Any form entries will be lost!"
#define BAD_FORM_MAILTO "Malformed mailto form submission!  Cancelled!"
#define FORM_MAILTO_DISALLOWED "Mail disallowed!  Cannot submit."
#define FORM_MAILTO_FAILED "Mailto form submission failed!"
#define FORM_MAILTO_CANCELLED "Mailto form submission Cancelled!!!"
#define SENDING_FORM_CONTENT "Sending form content..."
#define NO_ADDRESS_IN_MAILTO_URL "No email address is present in mailto URL!"
#define MAILTO_URL_TEMPOPEN_FAILED \
 "Unable to open temporary file for mailto URL!"
#define COMMENT_REQUEST_CANCELLED "Comment request cancelled!!!"
#define INC_ORIG_MSG_PROMPT \
 "Do you wish to include the original message? (y/n) "
#define SPAWNING_EDITOR_FOR_MAIL \
 "Spawning your selected editor to edit mail message"
#define ERROR_SPAWNING_EDITOR \
 "Error spawning editor, check your editor definition in the options menu"
#define SEND_COMMENT_PROMPT "Send this comment? (y/n) "
#define SENDING_YOUR_MSG "Sending your message..."
#define FILE_ACTIONS_DISALLOWED "file: ACTIONs are disallowed!"
#define FILE_SERVED_LINKS_DISALLOWED \
 "file: URLs via served links are disallowed!"
#define FILE_BOOKMARKS_DISALLOWED "file: URLs via bookmarks are disallowed!"
#define RETURN_TO_LYNX "Press <return> to return to Lynx."
#ifdef VMS
#define SPAWNING_MSG \
 "Spawning DCL subprocess.  Use 'logout' to return to Lynx.\n"
#else
#define SPAWNING_MSG \
 "Spawning your default shell.  Use 'exit' to return to Lynx.\n"
#endif /* VMS */
#define SPAWNING_DISABLED "Spawning is currently disabled."
#define DOWNLOAD_DISABLED "The 'd'ownload command is currently disabled."
#define NO_DOWNLOAD_INPUT "You cannot download a input field."
#define NO_DOWNLOAD_MAILTO_ACTION "Form has a mailto action!  Cannot download."
#define NO_DOWNLOAD_PRINT_OP "You cannot download a printing option."
#define NO_DOWNLOAD_UPLOAD_OP "You cannot download an upload option."
#define NO_DOWNLOAD_PERMIT_OP "You cannot download an permit option."
#define NO_DOWNLOAD_CHOICE "Nothing to download."
#define TRACE_ON "Trace ON!"
#define TRACE_OFF "Trace OFF!"
#define CLICKABLE_IMAGES_ON \
 "Links will be included for all images!  Reloading..."
#define CLICKABLE_IMAGES_OFF \
 "Standard image handling restored!  Reloading..."
#define PSEUDO_INLINE_ALTS_ON \
 "Pseudo_ALTs will be inserted for inlines without ALT strings!  Reloading..."
#define PSEUDO_INLINE_ALTS_OFF \
 "Inlines without an ALT string specified will be ignored!  Reloading..."
#define RAWMODE_OFF "Raw 8-bit or CJK mode toggled OFF!  Reloading..."
#define RAWMODE_ON "Raw 8-bit or CJK mode toggled ON!  Reloading..."
#define HEAD_D_L_OR_CANCEL \
 "Send HEAD request for D)ocument or L)ink, or C)ancel? (d,l,c): "
#define HEAD_D_OR_CANCEL \
 "Send HEAD request for D)ocument, or C)ancel? (d,c): "
#define DOC_NOT_HTTP_URL "Sorry, the document is not an http URL."
#define LINK_NOT_HTTP_URL "Sorry, the link is not an http URL."
#define FORM_ACTION_DISABLED "Sorry, the ACTION for this form is disabled."
#define FORM_ACTION_NOT_HTTP_URL \
 "Sorry, the ACTION for this form is not an http URL."
#define NOT_HTTP_URL_OR_ACTION "Not an http URL or form ACTION!"
#define NOT_IN_STARTING_REALM "URL is not in starting realm!"
#define NEWSPOSTING_DISABLED "News posting is disabled!"
#define DIRED_DISABLED "File management support is disabled!"
#define NO_JUMPFILE "No jump file is currently available."
#define JUMP_PROMPT "Jump to (use '?' for list): "
#define JUMP_DISALLOWED "Jumping to a shortcut URL is disallowed!"
#define RANDOM_URL_DISALLOWED "Random URL is disallowed!  Use a shortcut."
#define NO_RANDOM_URLS_YET "No random URLs have been used thus far."
#define BOOKMARKS_DISABLED "Bookmark features are currently disabled."
#define BOOKMARK_EXEC_DISABLED "Execution via bookmarks is disabled."
#define BOOKMARK_FILE_NOT_DEFINED \
 "Bookmark file is not defined. Use %s to see options."
#define NO_TEMP_FOR_HOTLIST \
 "Unable to open tempfile for X Mosaic hotlist conversion."
#define BOOKMARK_OPEN_FAILED "ERROR - unable to open bookmark file."
#define BOOKMARK_OPEN_FAILED_FOR_DEL \
 "Unable to open bookmark file for deletion of link."
#ifdef VMS
#define BOOKSCRA_OPEN_FAILED_FOR_DEL \
 "Unable to open scratch file for deletion of link."
#define ERROR_RENAMING_SCRA "Error renaming scratch file."
#else
#define BOOKTEMP_OPEN_FAILED_FOR_DEL \
 "Unable to open temporary file for deletion of link."
#define ERROR_RENAMING_TEMP "Error renaming temporary file."
#define BOOKTEMP_REOPEN_FAIL_FOR_DEL \
 "Unable to reopen temporary file for deletion of link."
#endif /* VMS */
#define BOOKMARK_LINK_NOT_ONE_LINE \
 "Link is not by itself all on one line in bookmark file."
#define BOOKMARK_DEL_FAILED "Bookmark deletion failed."
#define BOOKMARKS_NOT_TRAVERSED \
 "Bookmark files cannot be traversed (only http URLs)."
#define BOOKMARKS_NOT_OPEN \
 "Unable to open bookmark file, use 'a' to save a link first"
#define BOOK_D_L_OR_CANCEL \
 "Save D)ocument or L)ink to bookmark file or C)ancel? (d,l,c): "
#define BOOK_D_OR_CANCEL "Save D)ocument to bookmark file or C)ancel? (d,c): "
#define BOOK_L_OR_CANCEL "Save L)ink to bookmark file or C)ancel? (l,c): "
#define NOBOOK_POST_FORM \
 "Documents from forms with POST content cannot be saved as bookmarks."
#define NOBOOK_FORM_FIELD "Cannot save form fields/links"
#define NOBOOK_HSML \
 "History, showinfo, menu and list files cannot be saved as bookmarks."
#define CONFIRM_BOOKMARK_DELETE \
 "Do you really want to delete this link from your bookmark file? (y/n)"
#define MALFORMED_ADDRESS "Malformed address."
#define HISTORICAL_ON_MINIMAL_OFF \
 "Historical comment parsing ON (Minimal is overridden)!"
#define HISTORICAL_OFF_MINIMAL_ON \
 "Historical comment parsing OFF (Minimal is in effect)!"
#define HISTORICAL_ON_VALID_OFF \
 "Historical comment parsing ON (Valid is overridden)!"
#define HISTORICAL_OFF_VALID_ON \
 "Historical comment parsing OFF (Valid is in effect)!"
#define MINIMAL_ON_IN_EFFECT \
 "Minimal comment parsing ON (and in effect)!"
#define MINIMAL_OFF_VALID_ON \
 "Minimal comment parsing OFF (Valid is in effect)!"
#define MINIMAL_ON_BUT_HISTORICAL \
 "Minimal comment parsing ON (but Historical is in effect)!"
#define MINIMAL_OFF_HISTORICAL_ON \
 "Minimal comment parsing OFF (Historical is in effect)!"
#define SOFT_DOUBLE_QUOTE_ON "Soft double-quote parsing ON!"
#define SOFT_DOUBLE_QUOTE_OFF "Soft double-quote parsing OFF!"
#define ALREADY_AT_END "You are already at the end of this document."
#define ALREADY_AT_BEGIN "You are already at the beginning of this document."
#define ALREADY_AT_FIRST "You are already at the first document"
#define NO_LINKS_ABOVE "There are no links above this line of the document."
#define NO_LINKS_BELOW "There are no links below this line of the document."
#define MAXLEN_REACHED_DEL_OR_MOV \
 "Maximum length reached!  Delete text or move off field."
#define NOT_ON_SUBMIT_OR_LINK \
 "You are not on a form submission button or normal link."
#define NEED_CHECKED_RADIO_BUTTON \
 "One radio button must be checked at all times!"
#define PREV_DOC_QUERY "Do you want to go back to the previous document? [N]"
#define ARROWS_OR_TAB_TO_MOVE "Use arrows or tab to move off of field."
#define ENTER_TEXT_ARROWS_OR_TAB \
 "Enter text. Use arrows or tab to move off of field."
#define NO_FORM_ACTION "** Bad HTML!!  No form action defined. **"
#define BAD_HTML_NO_POPUP "Bad HTML!!  Unable to create popup window!"
#define POPUP_FAILED "Unable to create popup window!"
#define GOTO_DISALLOWED "Goto a random URL is disallowed!"
#define GOTO_NON_HTTP_DISALLOWED "Goto a non-http URL is disallowed!"
#define GOTO_CSO_DISALLOWED "You are not allowed to goto \"cso:\" URLs"
#define GOTO_FILE_DISALLOWED "You are not allowed to goto \"file:\" URLs"
#define GOTO_FINGER_DISALLOWED "You are not allowed to goto \"finger:\" URLs"
#define GOTO_FTP_DISALLOWED "You are not allowed to goto \"ftp:\" URLs"
#define GOTO_GOPHER_DISALLOWED "You are not allowed to goto \"gopher:\" URLs"
#define GOTO_HTTP_DISALLOWED "You are not allowed to goto \"http:\" URLs"
#define GOTO_HTTPS_DISALLOWED "You are not allowed to goto \"https:\" URLs"
#define GOTO_CGI_DISALLOWED "You are not allowed to goto \"lynxcgi:\" URLs"
#define GOTO_EXEC_DISALLOWED "You are not allowed to goto \"lynxexec:\" URLs"
#define GOTO_PROG_DISALLOWED "You are not allowed to goto \"lynxprog:\" URLs"
#define GOTO_MAILTO_DISALLOWED "You are not allowed to goto \"mailto:\" URLs"
#define GOTO_NEWS_DISALLOWED "You are not allowed to goto \"news:\" URLs"
#define GOTO_NNTP_DISALLOWED "You are not allowed to goto \"nntp:\" URLs"
#define GOTO_RLOGIN_DISALLOWED "You are not allowed to goto \"rlogin:\" URLs"
#define GOTO_SNEWS_DISALLOWED "You are not allowed to goto \"snews:\" URLs"
#define GOTO_TELNET_DISALLOWED "You are not allowed to goto \"telnet:\" URLs"
#define GOTO_TN3270_DISALLOWED "You are not allowed to goto \"tn3270:\" URLs"
#define GOTO_WAIS_DISALLOWED "You are not allowed to goto \"wais:\" URLs"
#define URL_TO_OPEN "URL to open: "
#define EDIT_CURRENT_GOTO "Edit the current Goto URL: "
#define EDIT_THE_PREV_GOTO "Edit the previous Goto URL: "
#define EDIT_A_PREV_GOTO "Edit a previous Goto URL: "
#define ENTER_DATABASE_QUERY "Enter a database query: "
#define ENTER_WHEREIS_QUERY "Enter a whereis query: "
#define EDIT_CURRENT_QUERY "Edit the current query: "
#define EDIT_THE_PREV_QUERY "Edit the previous query: "
#define EDIT_A_PREV_QUERY "Edit a previous query: "
#define USE_C_R_TO_RESUB_CUR_QUERY \
 "Use Control-R to resubmit the current query."
#define EDIT_CURRENT_SHORTCUT "Edit the current shortcut: "
#define EDIT_THE_PREV_SHORTCUT "Edit the previous shortcut: "
#define EDIT_A_PREV_SHORTCUT "Edit a previous shortcut: "
#define KEY_NOT_MAPPED_TO_JUMP_FILE "Key '%c' is not mapped to a jump file!"
#define CANNOT_LOCATE_JUMP_FILE "Cannot locate jump file!"
#define CANNOT_OPEN_JUMP_FILE "Cannot open jump file!"
#define ERROR_READING_JUMP_FILE "Error reading jump file!"
#define OUTOF_MEM_FOR_JUMP_FILE "Out of memory reading jump file!"
#define OUTOF_MEM_FOR_JUMP_TABLE "Out of memory reading jump table!"
#define NO_INDEX_FILE "No index is currently available."
#define CONFIRM_MAIN_SCREEN \
 "Do you really want to go to the Main screen? (y/n) [n] "
#define IN_MAIN_SCREEN "You are already at main screen!"
#define NOT_ISINDEX \
 "Not a searchable indexed document -- press '/' to search for a text string"
#define NO_OWNER \
 "No owner is defined for this file so you cannot send a comment"
#define NO_OWNER_USE "No owner is defined. Use %s? [N] "
#define CONFIRM_COMMENT "Do you wish to send a comment? [N]"
#define MAIL_DISALLOWED "Mail is disallowed so you cannot send a comment"
#define EDIT_DISABLED "The 'e'dit command is currently disabled."
#define NO_STATUS "System error - failure to get status."
#define NO_EDITOR "No editor is defined!"
#define PRINT_DISABLED "The 'p'rint command is currently disabled."
#define NO_TOOLBAR "Document has no Toolbar links or Banner."
#define NOOPEN_TRAV_ERR_FILE "Unable to open traversal errors output file"
#define FOLLOW_LINK_NUMBER "Follow link number: "
#define BAD_HTML_USE_TRACE "** Bad HTML!!  Use -trace to diagnose. **"
#define CANNOT_OPEN_TEMP "Can't open temporary file!"
#define CANNOT_OPEN_OUTPUT "Can't open output file!  Cancelling!"
#define EXECUTION_DISABLED "Execution is disabled."
#define EXECUTION_DISABLED_FOR_FILE \
 "Execution is not enabled for this file. See the Options menu (use %s)."
#define EXECUTION_NOT_COMPILED \
 "Execution capabilities are not compiled into this version."
#define CANNOT_DISPLAY_FILE "This file cannot be displayed on this terminal."
#define CANNOT_DISPLAY_FILE_D_OR_C \
 "This file cannot be displayed on this terminal:  D)ownload, or C)ancel"
#define WRONG_CHARSET_D_OR_C "%s  D)ownload, or C)ancel"
#define UNMAPPED_TYPE_D_OR_C "%s  D)ownload, or C)ancel"
#define CANCELLING_FILE "Cancelling file."
#define RETRIEVING_FILE "Retrieving file.  - PLEASE WAIT -"
#define FILENAME_PROMPT "Enter a filename: "
#define EDIT_THE_PREV_FILENAME "Edit the previous filename: "
#define EDIT_A_PREV_FILENAME "Edit a previous filename: "
#define NEW_FILENAME_PROMPT "Enter a new filename: "
#define FILENAME_CANNOT_BE_DOT "File name may not begin with a dot."
#ifdef VMS
#define FILE_EXISTS_HPROMPT "File exists. Create higher version? (y/n)"
#else
#define FILE_EXISTS_OPROMPT "File exists. Overwrite? (y/n)"
#endif /* VMS */
#define CANNOT_WRITE_TO_FILE "Cannot write to file."
#define MISCONF_DOWNLOAD_COMMAND "ERROR! - download command is misconfigured."
#define CANNOT_DOWNLOAD_FILE "Unable to download file."
#define SAVING "Saving..."
#define COULD_NOT_ACCESS_FILE "Could not access file."
#define CANNOT_EDIT_REMOTE_FILES \
 "Lynx cannot currently (E)dit remote WWW files"
#define NOAUTH_TO_EDIT_FILE "You are not authorized to edit this file."
#define TITLE_PROMPT "Title: "
#define SUBJECT_PROMPT "Subject: "
#define USERNAME_PROMPT "Username: "
#define PASSWORD_PROMPT "Password: "
#define USERNAME_PASSWORD_REQUIRED "lynx: Username and Password required!!!"
#define PASSWORD_REQUIRED "lynx: Password required!!!"
#define CGI_DISABLED "cgi support has been disabled by system administrator."
#define CGI_NOT_COMPILED \
 "Lynxcgi capabilities are not compiled into this version."
#define CANNOT_CONVERT_I_TO_O "Sorry, no known way of converting %s to %s."
#define CONNECT_SET_FAILED "Unable to set up connection."
#define CONNECT_FAILED "Unable to make connection"
#define MALFORMED_EXEC_REQUEST \
 "Executable link rejected due to malformed request."
#define BADCHAR_IN_EXEC_LINK \
 "Executable link rejected due to `%c' character."
#define BADLOCPATH_IN_EXEC_LINK \
 "Executable link rejected due to location or path."
#define MAIL_DISABLED "Mail access is disabled!"
#define ACCESS_ONLY_LOCALHOST \
 "Only files and servers on the local host can be accessed."
#define TELNET_DISABLED "Telnet access is disabled!"
#define TELNET_PORT_SPECS_DISABLED \
 "Telnet port specifications are disabled."
#define NEWS_DISABLED "USENET news access is disabled!"
#define RLOGIN_DISABLED "Rlogin access is disabled!"
#define FTP_DISABLED "Ftp access is disabled!"
#define NO_REFS_FROM_DOC "There are no references from this document."
#ifdef VMS
#define CANNOT_OPEN_COMFILE "Unable to open command file."
#endif /* VMS */
#define NEWS_POST_CANCELLED "News Post Cancelled!!!"
#define SPAWNING_EDITOR_FOR_NEWS \
 "Spawning your selected editor to edit news message"
#define POST_MSG_PROMPT "Post this message? (y/n) "
#ifdef VMS
#define HAVE_UNREAD_MAIL_MSG "*** You have unread mail. ***"
#else
#define HAVE_MAIL_MSG "*** You have mail. ***"
#endif /* VMS */
#define HAVE_NEW_MAIL_MSG "*** You have new mail. ***"
#define SAVE_REQUEST_CANCELLED "Save request cancelled!!!"
#define MAIL_REQUEST_CANCELLED "Mail request cancelled!!!"
#define MAILING_FILE "Mailing file.  Please wait..."
#define MAIL_REQUEST_FAILED "ERROR - Unable to mail file"
#define CONFIRM_LONG_SCREEN_PRINT \
 "File is %d screens long.  Are you sure you want to print? [y]"
#define PRINT_REQUEST_CANCELLED "Print request cancelled!!!"
#define PRESS_RETURN_TO_BEGIN "Press <return> to begin: "
#define PRESS_RETURN_TO_FINISH "Press <return> to finish: "
#define CONFIRM_LONG_PAGE_PRINT \
 "File is %d pages long.  Are you sure you want to print? [y]"
#define FILE_ALLOC_FAILED "ERROR - Unable to allocate file space!!!"
#define UNABLE_TO_OPEN_TEMPFILE "Unable to open tempfile"
#define UNABLE_TO_OPEN_PRINTOP_FILE "Unable to open print options file"
#define PRINTING_FILE "Printing file.  Please wait..."
#define MAIL_ADDRESS_PROMPT "Please enter a valid internet mail address: "
#define PRINTER_MISCONF_ERROR "ERROR! - printer is misconfigured!"
#define MISDIRECTED_MAP_REQUEST "Misdirected client-side image MAP request!"
#define MAP_NOT_ACCESSIBLE "Client-side image MAP is not accessible!"
#define MAPS_NOT_AVAILABLE "No client-side image MAPs are available!"
#define MAP_NOT_AVAILABLE "Client-side image MAP is not available!"
#define OPTION_SCREEN_NEEDS_24 \
 "Screen height must be at least 24 lines for the Options menu!"
#define OPTION_SCREEN_NEEDS_23 \
 "Screen height must be at least 23 lines for the Options menu!"
#define OPTION_SCREEN_NEEDS_22 \
 "Screen height must be at least 22 lines for the Options menu!"
#define COMMAND_PROMPT "Command: "
#define SELECT_SEGMENT "Select "
#define CAP_LETT_SEGMENT "capital letter"
#define OF_OPT_LINE_SEGMENT " of option line,"
#define TO_SAVE_SEGMENT " to save,"
#define OR_SEGMENT " or "
#define TO_RETURN_SEGMENT " to return to Lynx."
#define ACCEPT_DATA "Hit return to accept entered data."
#define ACCEPT_DATA_OR_DEFAULT \
"Hit return to accept entered data.  Delete data to invoke the default."
#define VALUE_ACCEPTED "Value accepted!"
#define VALUE_ACCEPTED_WARNING_X \
 "Value accepted! -- WARNING: Lynx is configured for XWINDOWS!"
#define VALUE_ACCEPTED_WARNING_NONX \
 "Value accepted! -- WARNING: Lynx is NOT configured for XWINDOWS!"
#define EDITOR_LOCKED "You are not allowed to change which editor to use!"
#define FAILED_TO_SET_DISPLAY "Failed to set DISPLAY variable!"
#define FAILED_CLEAR_SET_DISPLAY "Failed to clear DISPLAY variable!"
#define BOOKMARK_CHANGE_DISALLOWED \
 "You are not allowed to change the bookmark file!"
#define DOTFILE_ACCESS_DISABLED "Access to dot files is disabled!"
#define UA_COPYRIGHT_WARNING \
 "WARNING: Misrepresentation of the User-Agent may be a copyright violation!"
#define CHANGE_OF_SETTING_DISALLOWED \
 "You are not allowed to change this setting."
#define SAVING_OPTIONS "Saving Options..."
#define OPTIONS_SAVED "Options saved!"
#define OPTIONS_NOT_SAVED "Unable to save Options!"
#define R_TO_RETURN_TO_LYNX " 'r' to return to Lynx "
#define SAVE_OR_R_TO_RETURN_TO_LYNX " '>' to save, or 'r' to return to Lynx "
#define ANY_KEY_CHANGE_RET_ACCEPT \
 "Hit any key to change value; RETURN to accept: "
#define ERROR_UNCOMPRESSING_TEMP "Error uncompressing temporary file!"
#define UNSUPPORTED_URL_SCHEME "Unsupported URL scheme!"
#define UNSUPPORTED_DATA_URL "Unsupported data: URL!  Use SHOWINFO, for now."
#define	SERVER_ASKED_FOR_REDIRECTION \
 "Server asked for redirection of POST content to"
#define	PROCEED_GET_CANCEL "P)roceed, use G)ET or C)ancel "
#define	ADVANCED_POST_REDIRECT \
 "Redirection of POST content. P)roceed, see U)RL, use G)ET or C)ancel"
#define	LOCATION_HEADER "Location: "
#define STRING_NOT_FOUND "'%s' not found!"
#define MULTIBOOKMARKS_DEFAULT "Default Bookmark File"
#define MULTIBOOKMARKS_SAVE "Select destination or ^G to Cancel: "
#define MULTIBOOKMARKS_SMALL "Screen too small! (8x35 min)"
#define MULTIBOOKMARKS_MOVE "'[' previous, ']' next screen"
#define MULTIBOOKMARKS_SELF \
 "Reproduce L)ink in this bookmark file or C)ancel? (l,c): "
#define MULTIBOOKMARKS_DISALLOWED "Multiple bookmark support is not available."

#ifdef DIRED_SUPPORT
#define DIRED_NOVICELINE \
 "  C)reate  D)ownload  E)dit  F)ull menu  M)odify  R)emove  T)ag  U)pload     \n"
#define CURRENT_LINK_STATUS_FAILED "Failed to obtain status of current link!"
#endif /* DIRED_SUPPORT */

#endif /* LYMESSAGES_EN_H */
