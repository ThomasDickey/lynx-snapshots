/* $LynxId: LYDownload.c,v 1.70 2018/05/11 22:57:30 tom Exp $ */
#include <HTUtils.h>
#include <HTParse.h>
#include <HTList.h>
#include <HTAlert.h>
#include <LYCurses.h>
#include <LYUtils.h>
#include <LYGlobalDefs.h>
#include <LYStrings.h>
#include <LYDownload.h>

#include <LYLeaks.h>

/*
 * LYDownload takes a URL and downloads it using a user selected download
 * program
 *
 * It parses an incoming link that looks like
 *
 * LYNXDOWNLOAD://Method=<#>/File=<STRING>/SugFile=<STRING>
 */
#ifdef VMS
BOOLEAN LYDidRename = FALSE;
#endif /* VMS */

static char LYValidDownloadFile[LY_MAXPATH] = "\0";

void LYDownload(char *line)
{
    char *Line = NULL, *method, *file, *sug_file = NULL;
    int method_number;
    int count;
    char *the_command = 0;
    bstring *buffer = NULL;
    bstring *command = NULL;
    char *cp;
    lynx_list_item_type *download_command = 0;
    int ch;
    RecallType recall;
    int FnameTotal;
    int FnameNum;
    BOOLEAN FirstRecall = TRUE;
    BOOLEAN SecondS = FALSE;

#ifdef VMS
    LYDidRename = FALSE;
#endif /* VMS */

    /*
     * Make sure we have a valid download file comparison string loaded via the
     * download options menu.  - FM
     */
    if (LYValidDownloadFile[0] == '\0') {
	goto failed;
    }

    /*
     * Make a copy of the LYNXDOWNLOAD internal URL for parsing.  - FM
     */
    if (StrAllocCopy(Line, line) == 0)
	goto failed;

    /*
     * Parse out the File, sug_file, and the Method.
     */
    if ((file = strstr(Line, "/File=")) == NULL)
	goto failed;
    *file = '\0';
    /*
     * Go past "File=".
     */
    file += 6;

    if ((sug_file = strstr(file + 1, "/SugFile=")) != NULL) {
	*sug_file = '\0';
	/*
	 * Go past "SugFile=".
	 */
	sug_file += 9;
	HTUnEscape(sug_file);
    }

    /*
     * Make sure that the file string is the one from the last displayed
     * download options menu.  - FM
     */
    if (strcmp(file, LYValidDownloadFile)) {
	goto failed;
    }
#if defined(DIRED_SUPPORT)
    /* FIXME: use HTLocalName */
    if (!StrNCmp(file, "file://localhost", 16)) {
#ifdef __DJGPP__
	if (!StrNCmp(file + 16, "/dev/", 5))
	    file += 16;
	else {
	    file += 17;
	    file = HTDOS_name(file);
	}
#else
	file += 16;
#endif /* __DJGPP__ */
    } else if (isFILE_URL(file))
	file += LEN_FILE_URL;
    HTUnEscape(file);
#else
#if defined(_WINDOWS)		/* 1997/10/15 (Wed) 16:27:38 */
    if (!StrNCmp(file, "file://localhost/", 17))
	file += 17;
    else if (!StrNCmp(file, "file:/", 6))
	file += 6;
    HTUnEscape(file);
#endif /* _WINDOWS */
#endif /* DIRED_SUPPORT */

    if ((method = strstr(Line, "Method=")) == NULL)
	goto failed;
    /*
     * Go past "Method=".
     */
    method += 7;
    method_number = atoi(method);

    /*
     * Set up the sug_filenames recall buffer.
     */
    FnameTotal = (sug_filenames ? HTList_count(sug_filenames) : 0);
    recall = ((FnameTotal >= 1) ? RECALL_URL : NORECALL);
    FnameNum = FnameTotal;

    if (method_number < 0) {
	/*
	 * Write to local file.
	 */
	_statusline(FILENAME_PROMPT);
      retry:
	if (sug_file) {
	    BStrCopy0(buffer, sug_file);
	} else {
	    BStrCopy0(buffer, "");
	}

      check_recall:
	if ((ch = LYgetBString(&buffer, FALSE, 0, recall)) < 0 ||
	    isBEmpty(buffer) ||
	    ch == UPARROW_KEY ||
	    ch == DNARROW_KEY) {

	    if (recall && ch == UPARROW_KEY) {
		if (FirstRecall) {
		    FirstRecall = FALSE;
		    /*
		     * Use the last Fname in the list.  - FM
		     */
		    FnameNum = 0;
		} else {
		    /*
		     * Go back to the previous Fname in the list.  - FM
		     */
		    FnameNum++;
		}
		if (FnameNum >= FnameTotal) {
		    /*
		     * Reset the FirstRecall flag, and use sug_file or a blank.
		     * - FM
		     */
		    FirstRecall = TRUE;
		    FnameNum = FnameTotal;
		    _statusline(FILENAME_PROMPT);
		    goto retry;
		} else if ((cp = (char *) HTList_objectAt(sug_filenames,
							  FnameNum)) != NULL) {
		    BStrCopy0(buffer, cp);
		    if (FnameTotal == 1) {
			_statusline(EDIT_THE_PREV_FILENAME);
		    } else {
			_statusline(EDIT_A_PREV_FILENAME);
		    }
		    goto check_recall;
		}
	    } else if (recall && ch == DNARROW_KEY) {
		if (FirstRecall) {
		    FirstRecall = FALSE;
		    /*
		     * Use the first Fname in the list.  - FM
		     */
		    FnameNum = FnameTotal - 1;
		} else {
		    /*
		     * Advance to the next Fname in the list.  - FM
		     */
		    FnameNum--;
		}
		if (FnameNum < 0) {
		    /*
		     * Set the FirstRecall flag, and use sug_file or a blank.
		     * - FM
		     */
		    FirstRecall = TRUE;
		    FnameNum = FnameTotal;
		    _statusline(FILENAME_PROMPT);
		    goto retry;
		} else if ((cp = (char *) HTList_objectAt(sug_filenames,
							  FnameNum)) != NULL) {
		    BStrCopy0(buffer, cp);
		    if (FnameTotal == 1) {
			_statusline(EDIT_THE_PREV_FILENAME);
		    } else {
			_statusline(EDIT_A_PREV_FILENAME);
		    }
		    goto check_recall;
		}
	    }

	    /*
	     * Save cancelled.
	     */
	    goto cancelled;
	}

	BStrCopy(command, buffer);
	if (!LYValidateFilename(&buffer, &command))
	    goto cancelled;
#ifdef HAVE_POPEN
	else if (LYIsPipeCommand(buffer->str)) {
	    /* I don't know how to download to a pipe */
	    HTAlert(CANNOT_WRITE_TO_FILE);
	    _statusline(NEW_FILENAME_PROMPT);
	    FirstRecall = TRUE;
	    FnameNum = FnameTotal;
	    goto retry;
	}
#endif

	/*
	 * See if it already exists.
	 */
	switch (LYValidateOutput(buffer->str)) {
	case 'Y':
	    break;
	case 'N':
	    _statusline(NEW_FILENAME_PROMPT);
	    FirstRecall = TRUE;
	    FnameNum = FnameTotal;
	    goto retry;
	default:
	    goto cleanup;
	}

	/*
	 * See if we can write to it.
	 */
	CTRACE((tfp, "LYDownload: filename is %s\n", buffer->str));

	SecondS = TRUE;

	HTInfoMsg(SAVING);
#ifdef VMS
	/*
	 * Try rename() first.  - FM
	 */
	CTRACE((tfp, "command: rename(%s, %s)\n", file, buffer->str));
	if (rename(file, buffer->str)) {
	    /*
	     * Failed.  Use spawned COPY_COMMAND.  - FM
	     */
	    CTRACE((tfp, "         FAILED!\n"));
	    LYCopyFile(file, buffer->str);
	} else {
	    /*
	     * We don't have the temporary file (it was renamed to a permanent
	     * file), so set a flag to pop out of the download menu.  - FM
	     */
	    LYDidRename = TRUE;
	}
	chmod(buffer->str, HIDE_CHMOD);
#else /* Unix: */

	LYCopyFile(file, buffer->str);
	LYRelaxFilePermissions(buffer->str);
#endif /* VMS */

    } else {
	/*
	 * Use configured download commands.
	 */
	BStrCopy0(buffer, "");
	for (count = 0, download_command = downloaders;
	     count < method_number;
	     count++, download_command = download_command->next) ;	/* null body */

	/*
	 * Commands have the form "command %s [etc]" where %s is the filename.
	 */
	if (download_command->command != NULL) {
	    /*
	     * Check for two '%s' and ask for the local filename if there is.
	     */
	    if (HTCountCommandArgs(download_command->command) >= 2) {
		_statusline(FILENAME_PROMPT);

	      again:
		if (sug_file) {
		    BStrCopy0(buffer, sug_file);
		} else {
		    BStrCopy0(buffer, "");
		}

	      check_again:
		if ((ch = LYgetBString(&buffer, FALSE, 0, recall)) < 0 ||
		    isBEmpty(buffer) ||
		    ch == UPARROW_KEY ||
		    ch == DNARROW_KEY) {

		    if (recall && ch == UPARROW_KEY) {
			if (FirstRecall) {
			    FirstRecall = FALSE;
			    /*
			     * Use the last Fname in the list.  - FM
			     */
			    FnameNum = 0;
			} else {
			    /*
			     * Go back to the previous Fname in the list.  - FM
			     */
			    FnameNum++;
			}
			if (FnameNum >= FnameTotal) {
			    /*
			     * Reset the FirstRecall flag, and use sug_file or
			     * a blank.  - FM
			     */
			    FirstRecall = TRUE;
			    FnameNum = FnameTotal;
			    _statusline(FILENAME_PROMPT);
			    goto again;
			} else if ((cp = (char *) HTList_objectAt(sug_filenames,
								  FnameNum))
				   != NULL) {
			    BStrCopy0(buffer, cp);
			    if (FnameTotal == 1) {
				_statusline(EDIT_THE_PREV_FILENAME);
			    } else {
				_statusline(EDIT_A_PREV_FILENAME);
			    }
			    goto check_again;
			}
		    } else if (recall && ch == DNARROW_KEY) {
			if (FirstRecall) {
			    FirstRecall = FALSE;
			    /*
			     * Use the first Fname in the list.  - FM
			     */
			    FnameNum = FnameTotal - 1;
			} else {
			    /*
			     * Advance to the next Fname in the list.  - FM
			     */
			    FnameNum--;
			}
			if (FnameNum < 0) {
			    /*
			     * Set the FirstRecall flag, and use sug_file or a
			     * blank.  - FM
			     */
			    FirstRecall = TRUE;
			    FnameNum = FnameTotal;
			    _statusline(FILENAME_PROMPT);
			    goto again;
			} else if ((cp = (char *) HTList_objectAt(sug_filenames,
								  FnameNum))
				   != NULL) {
			    BStrCopy0(buffer, cp);
			    if (FnameTotal == 1) {
				_statusline(EDIT_THE_PREV_FILENAME);
			    } else {
				_statusline(EDIT_A_PREV_FILENAME);
			    }
			    goto check_again;
			}
		    }

		    /*
		     * Download cancelled.
		     */
		    goto cancelled;
		}

		if (no_dotfiles || !show_dotfiles) {
		    if (*LYPathLeaf(buffer->str) == '.') {
			HTAlert(FILENAME_CANNOT_BE_DOT);
			_statusline(NEW_FILENAME_PROMPT);
			goto again;
		    }
		}
		/*
		 * Cancel if the user entered "/dev/null" on Unix, or an "nl:"
		 * path on VMS.  - FM
		 */
		if (LYIsNullDevice(buffer->str)) {
		    goto cancelled;
		}
		SecondS = TRUE;
	    }

	    /*
	     * The following is considered a bug by the community.  If the
	     * command only takes one argument on the command line, then the
	     * suggested file name is not used.  It actually is not a bug at
	     * all and does as it should, putting both names on the command
	     * line.
	     */
	    count = 1;
	    HTAddParam(&the_command, download_command->command, count, file);
	    if (HTCountCommandArgs(download_command->command) > 1)
		HTAddParam(&the_command, download_command->command, ++count, buffer->str);
	    HTEndParam(&the_command, download_command->command, count);

	} else {
	    HTAlert(MISCONF_DOWNLOAD_COMMAND);
	    goto failed;
	}

	CTRACE((tfp, "command: %s\n", the_command));
	stop_curses();
	LYSystem(the_command);
	FREE(the_command);
	start_curses();
	/* don't remove(file); */
    }

    if (SecondS == TRUE) {
#ifdef VMS
	if (0 == strncasecomp(buffer->str, "sys$disk:", 9)) {
	    if (0 == StrNCmp((buffer->str + 9), "[]", 2)) {
		HTAddSugFilename(buffer->str + 11);
	    } else {
		HTAddSugFilename(buffer->str + 9);
	    }
	} else {
	    HTAddSugFilename(buffer->str);
	}
#else
	HTAddSugFilename(buffer->str);
#endif /* VMS */
    }
    goto cleanup;

  failed:
    HTAlert(CANNOT_DOWNLOAD_FILE);
    goto cleanup;

  cancelled:
    HTInfoMsg(CANCELLING);

  cleanup:
    FREE(Line);
    BStrFree(buffer);
    return;
}

/*
 * Compare a filename with a given suffix, which we have set to give a rough
 * idea of its content.
 */
static int SuffixIs(char *filename, const char *suffix)
{
    size_t have = strlen(filename);
    size_t need = strlen(suffix);

    return have > need && !strcmp(filename + have - need, suffix);
}

/*
 * LYdownload_options writes out the current download choices to a file so that
 * the user can select downloaders in the same way that they select all other
 * links.  Download links look like:
 * LYNXDOWNLOAD://Method=<#>/File=<STRING>/SugFile=<STRING>
 */
int LYdownload_options(char **newfile, char *data_file)
{
    static char tempfile[LY_MAXPATH] = "\0";
    char *downloaded_url = NULL;
    char *sug_filename = NULL;
    FILE *fp0;
    lynx_list_item_type *cur_download;
    int count;

    /*
     * Get a suggested filename.
     */
    StrAllocCopy(sug_filename, *newfile);
    change_sug_filename(sug_filename);

    if ((fp0 = InternalPageFP(tempfile, TRUE)) == 0)
	return (-1);

    StrAllocCopy(downloaded_url, *newfile);
    LYLocalFileToURL(newfile, tempfile);

    LYStrNCpy(LYValidDownloadFile,
	      data_file,
	      (sizeof(LYValidDownloadFile) - 1));
    LYforce_no_cache = TRUE;	/* don't cache this doc */

    BeginInternalPage(fp0, DOWNLOAD_OPTIONS_TITLE, DOWNLOAD_OPTIONS_HELP);

    fprintf(fp0, "<pre>\n");
    fprintf(fp0, "<em>%s</em> %s\n",
	    gettext("Downloaded link:"),
	    downloaded_url);
    FREE(downloaded_url);

    fprintf(fp0, "<em>%s</em> %s\n",
	    gettext("Suggested file name:"),
	    sug_filename);

    fprintf(fp0, "\n%s\n",
	    (user_mode == NOVICE_MODE)
	    ? gettext("Standard download options:")
	    : gettext("Download options:"));

    if (!no_disk_save) {
#if defined(DIRED_SUPPORT)
	/*
	 * Disable save to disk option for local files.
	 */
	if (!lynx_edit_mode)
#endif /* DIRED_SUPPORT */
	{
	    fprintf(fp0,
		    "   <a href=\"%s//Method=-1/File=%s/SugFile=%s%s\">%s</a>\n",
		    STR_LYNXDOWNLOAD,
		    data_file,
		    NonNull(lynx_save_space),
		    sug_filename,
		    gettext("Save to disk"));
	    /*
	     * If it is not a binary file, offer the opportunity to view the
	     * downloaded temporary file (see HTSaveToFile).
	     */
	    if (SuffixIs(data_file, HTML_SUFFIX)
		|| SuffixIs(data_file, TEXT_SUFFIX)) {
		char *target = NULL;
		char *source = LYAddPathToSave(data_file);

		LYLocalFileToURL(&target, source);
		fprintf(fp0,
			"   <a href=\"%s\">%s</a>\n",
			target,
			gettext("View temporary file"));

		FREE(source);
		FREE(target);
	    }
	}
    } else {
	fprintf(fp0, "   <em>%s</em>\n", gettext("Save to disk disabled."));
    }

    if (user_mode == NOVICE_MODE)
	fprintf(fp0, "\n%s\n", gettext("Local additions:"));

    if (downloaders != NULL) {
	for (count = 0, cur_download = downloaders; cur_download != NULL;
	     cur_download = cur_download->next, count++) {
	    if (!no_download || cur_download->always_enabled) {
		fprintf(fp0,
			"   <a href=\"%s//Method=%d/File=%s/SugFile=%s\">",
			STR_LYNXDOWNLOAD, count, data_file, sug_filename);
		fprintf(fp0, "%s", (cur_download->name
				    ? cur_download->name
				    : gettext("No Name Given")));
		fprintf(fp0, "</a>\n");
	    }
	}
    }

    fprintf(fp0, "</pre>\n");
    EndInternalPage(fp0);
    LYCloseTempFP(fp0);
    LYRegisterUIPage(*newfile, UIP_DOWNLOAD_OPTIONS);

    /*
     * Free off temp copy.
     */
    FREE(sug_filename);

    return (0);
}
