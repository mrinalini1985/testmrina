// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003 - Tim Kemp and Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "StdAfx.h"
#include "TortoiseProc.h"
#include "svn.h"
#include "UnicodeUtils.h"

SVN::SVN(void)
{
	apr_initialize();
	memset (&ctx, 0, sizeof (ctx));
	parentpool = svn_pool_create(NULL);
	svn_config_ensure(parentpool);
	pool = svn_pool_create (parentpool);

	// set up authentication

	/* The whole list of registered providers */
	apr_array_header_t *providers
		= apr_array_make (pool, 1, sizeof (svn_auth_provider_object_t *));

	/* Allocate all the provider objects. */
	svn_auth_provider_object_t *simple_prompt_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*simple_prompt_provider));
	svn_auth_provider_object_t *username_prompt_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*username_prompt_provider));
	svn_auth_provider_object_t *simple_wc_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*simple_wc_provider));
	svn_auth_provider_object_t *username_wc_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*username_wc_provider));
	svn_auth_provider_object_t *ssl_server_file_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*ssl_server_file_provider));
	svn_auth_provider_object_t *ssl_client_cred_file_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*ssl_client_cred_file_provider));
	svn_auth_provider_object_t *ssl_client_pw_file_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*ssl_client_pw_file_provider));
	svn_auth_provider_object_t *ssl_server_prompt_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*ssl_server_prompt_provider));
	svn_auth_provider_object_t *ssl_client_prompt_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*ssl_client_prompt_provider));
	svn_auth_provider_object_t *ssl_client_pw_prompt_provider = (svn_auth_provider_object_t *)apr_pcalloc (pool, sizeof(*ssl_client_pw_prompt_provider));

	/* The main disk-caching auth providers, for both
	'username/password' creds and 'username' creds.  */
	svn_client_get_simple_provider (&(simple_wc_provider->vtable), &(simple_wc_provider->provider_baton), pool);
	*(svn_auth_provider_object_t **)apr_array_push (providers) = simple_wc_provider;

	svn_client_get_username_provider(&(username_wc_provider->vtable), &(username_wc_provider->provider_baton), pool);
	*(svn_auth_provider_object_t **)apr_array_push (providers) = username_wc_provider;

	/* The server-cert, client-cert, and client-cert-password  providers. */
	svn_client_get_ssl_server_file_provider(&ssl_server_file_provider->vtable, &ssl_server_file_provider->provider_baton, pool);
	*(svn_auth_provider_object_t **)apr_array_push (providers) = ssl_server_file_provider;

	svn_client_get_ssl_client_file_provider(&ssl_client_cred_file_provider->vtable, &ssl_client_cred_file_provider->provider_baton, pool);
	*(svn_auth_provider_object_t **)apr_array_push (providers) = ssl_client_cred_file_provider;

	svn_client_get_ssl_pw_file_provider(&ssl_client_pw_file_provider->vtable, &ssl_client_pw_file_provider->provider_baton, pool);
	*(svn_auth_provider_object_t **)apr_array_push (providers) = ssl_client_pw_file_provider;

	/* Two prompting providers, one for username/password, one for
	just username. */

	svn_client_get_simple_prompt_provider(&(simple_prompt_provider->vtable),
		&(simple_prompt_provider->provider_baton),
		(svn_client_prompt_t)prompt, this,
		2, /* retry limit */ pool);

	svn_client_get_username_prompt_provider 
		(&(username_prompt_provider->vtable),
		&(username_prompt_provider->provider_baton),
		(svn_client_prompt_t)prompt, this,
		2, /* retry limit */ pool);

	/* Three prompting providers for server-certs, client-certs,
	and client-cert-passphrases.  */

	svn_client_get_ssl_server_prompt_provider
		(&ssl_server_prompt_provider->vtable,
		&ssl_server_prompt_provider->provider_baton,
		(svn_client_prompt_t)prompt, this,
		pool);

	svn_client_get_ssl_client_prompt_provider
		(&ssl_client_prompt_provider->vtable,
		&ssl_client_prompt_provider->provider_baton,
		(svn_client_prompt_t)prompt, this,
		pool);

	svn_client_get_ssl_pw_prompt_provider
		(&ssl_client_pw_prompt_provider->vtable,
		&ssl_client_pw_prompt_provider->provider_baton,
		(svn_client_prompt_t)prompt, this,
		pool);

	*(svn_auth_provider_object_t **)apr_array_push (providers) = simple_prompt_provider;
	*(svn_auth_provider_object_t **)apr_array_push (providers) = username_prompt_provider;       
	*(svn_auth_provider_object_t **)apr_array_push (providers) = ssl_server_prompt_provider;
	*(svn_auth_provider_object_t **)apr_array_push (providers) = ssl_client_prompt_provider;
	*(svn_auth_provider_object_t **)apr_array_push (providers) = ssl_client_pw_prompt_provider;

	/* Build an authentication baton to give to libsvn_client. */
	svn_auth_open (&auth_baton, providers, pool);



	ctx.auth_baton = auth_baton;
	ctx.prompt_func = (svn_client_prompt_t)prompt;
	ctx.prompt_baton = this;
	ctx.log_msg_func = svn_cl__get_log_message;
	ctx.log_msg_baton = logMessage("");
	ctx.notify_func = notify;
	ctx.notify_baton = this;
	ctx.cancel_func = cancel;
	ctx.cancel_baton = this;

	// set up the configuration
	svn_config_get_config (&(ctx.config), pool);

}

SVN::~SVN(void)
{
	svn_pool_destroy (pool);
	svn_pool_destroy (parentpool);
	apr_terminate();
}

BOOL SVN::Prompt(CString& info, CString prompt, BOOL hide) 
{
	CPromptDlg dlg;
	dlg.m_info = prompt;
	dlg.m_sPass = info;
	
	dlg.SetHide(hide);

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		info = dlg.m_sPass;
		if (dlg.m_saveCheck)
		{
			svn_auth_set_parameter(ctx.auth_baton, SVN_AUTH_PARAM_NO_AUTH_CACHE, NULL);
		}
		else
		{
			svn_auth_set_parameter(ctx.auth_baton, SVN_AUTH_PARAM_NO_AUTH_CACHE, (void *) "");
		}
		return TRUE;
	}
	if (nResponse == IDABORT)
	{
		//the prompt dialog box could not be shown!
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("TortoiseSVN"), MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
	} // if (nResponse == IDABORT)
	return FALSE;
}
BOOL SVN::Cancel() {return FALSE;};
BOOL SVN::Notify(CString path, svn_wc_notify_action_t action, svn_node_kind_t kind, CString myme_type, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, LONG rev) {return TRUE;};
BOOL SVN::Log(LONG rev, CString author, CString date, CString message, CString cpaths) {return TRUE;};

struct log_msg_baton
{
  const char *message;  /* the message. */
  const char *message_encoding; /* the locale/encoding of the message. */
  const char *base_dir; /* the base directory for an external edit. UTF-8! */
  const char *tmpfile_left; /* the tmpfile left by an external edit. UTF-8! */
  apr_pool_t *pool; /* a pool. */
};

CString SVN::GetLastErrorMessage()
{
	CString msg;
	if (Err != NULL)
	{
		msg = Err->message;
		while (Err->child)
		{
			Err = Err->child;
			msg += "\n";
			msg += Err->message;
		}
		return msg;
	}
	return _T("");
}

BOOL SVN::Checkout(CString moduleName, CString destPath, LONG revision, BOOL recurse)
{
	preparePath(destPath);

	Err = svn_client_checkout (	CUnicodeUtils::GetUTF8(moduleName),
								CUnicodeUtils::GetUTF8(destPath),
								getRevision (revision),
								recurse,
								&ctx,
								pool );

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Remove(CString path, BOOL force)
{
	preparePath(path);

	svn_client_commit_info_t *commit_info = NULL;

	Err = svn_client_delete (&commit_info, CUnicodeUtils::GetUTF8(path), force,
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Revert(CString path, BOOL recurse)
{
	preparePath(path);
	Err = svn_client_revert (CUnicodeUtils::GetUTF8(path), recurse, &ctx, pool);

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}


BOOL SVN::Add(CString path, BOOL recurse)
{
	preparePath(path);
	Err = svn_client_add (CUnicodeUtils::GetUTF8(path), recurse, &ctx, pool);

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Update(CString path, LONG revision, BOOL recurse)
{
	preparePath(path);
	Err = svn_client_update (CUnicodeUtils::GetUTF8(path),
							getRevision (revision),
							recurse,
							&ctx,
							pool);

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

LONG SVN::Commit(CString path, CString message, BOOL recurse)
{
	preparePath(path);
	svn_client_commit_info_t *commit_info = NULL;

	ctx.log_msg_baton = logMessage(CUnicodeUtils::GetUTF8(message));
	Err = svn_client_commit (&commit_info, 
							target ((LPCTSTR)path), 
							!recurse,
							&ctx,
							pool);
	ctx.log_msg_baton = logMessage("");
	if(Err != NULL)
	{
		return 0;
	}

	if(commit_info && SVN_IS_VALID_REVNUM (commit_info->revision))
		return commit_info->revision;
	return -1;
}

BOOL SVN::Copy(CString srcPath, CString destPath, LONG revision)
{
	preparePath(srcPath);
	preparePath(destPath);
	svn_client_commit_info_t *commit_info = NULL;

	Err = svn_client_copy (&commit_info,
							CUnicodeUtils::GetUTF8(srcPath),
							getRevision (revision),
							CUnicodeUtils::GetUTF8(destPath),
							NULL,
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Move(CString srcPath, CString destPath, BOOL force)
{
	preparePath(srcPath);
	preparePath(destPath);
	svn_client_commit_info_t *commit_info = NULL;

	Err = svn_client_move (&commit_info,
							CUnicodeUtils::GetUTF8(srcPath),
							getRevision (-1),
							CUnicodeUtils::GetUTF8(destPath),
							force,
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::MakeDir(CString path, CString message)
{
	preparePath(path);

	svn_client_commit_info_t *commit_info = NULL;

	Err = svn_client_mkdir (&commit_info,
							CUnicodeUtils::GetUTF8(path),
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::CleanUp(CString path)
{
	preparePath(path);
	Err = svn_client_cleanup (CUnicodeUtils::GetUTF8(path), &ctx, pool);

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Resolve(CString path, BOOL recurse)
{
	preparePath(path);
	Err = svn_client_resolve (CUnicodeUtils::GetUTF8(path),
							recurse,
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Export(CString srcPath, CString destPath, LONG revision)
{
	preparePath(srcPath);
	preparePath(destPath);

	Err = svn_client_export (CUnicodeUtils::GetUTF8(srcPath),
							CUnicodeUtils::GetUTF8(destPath),
							getRevision (revision),
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Switch(CString path, CString url, LONG revision, BOOL recurse)
{
	preparePath(path);
	preparePath(url);

	Err = svn_client_switch (CUnicodeUtils::GetUTF8(path),
							CUnicodeUtils::GetUTF8(url),
							getRevision (revision),
							recurse,
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Import(CString path, CString url, CString newEntry, CString message, BOOL recurse)
{
	preparePath(path);
	preparePath(url);

	svn_client_commit_info_t *commit_info = NULL;

	const char * newEntr = CUnicodeUtils::GetUTF8(newEntry);
	if (newEntry.IsEmpty())
		newEntr = NULL;
	ctx.log_msg_baton = logMessage(CUnicodeUtils::GetUTF8(message));
	Err = svn_client_import (&commit_info,
							CUnicodeUtils::GetUTF8(path),
							CUnicodeUtils::GetUTF8(url),
							newEntr,
							!recurse,
							&ctx,
							pool);
	ctx.log_msg_baton = logMessage("");
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Merge(CString path1, LONG revision1, CString path2, LONG revision2, CString localPath, BOOL force, BOOL recurse)
{
	preparePath(path1);
	preparePath(path2);
	preparePath(localPath);

	Err = svn_client_merge (CUnicodeUtils::GetUTF8(path1),
							getRevision (revision1),
							CUnicodeUtils::GetUTF8(path2),
							getRevision (revision2),
							CUnicodeUtils::GetUTF8(localPath),
							recurse,
							force,
							false,		//no 'dry-run'
							&ctx,
							pool);
	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::ReceiveLog(CString path, LONG revisionStart, LONG revisionEnd, BOOL changed)
{
	svn_opt_revision_t revEnd;
	memset (&revEnd, 0, sizeof (revEnd));
	revEnd.kind = svn_opt_revision_number;
	revEnd.value.number = revisionEnd;
	if(revisionEnd == -1)
	{
		revEnd.kind = svn_opt_revision_head;
		revisionEnd = 0;
	}
	else
	{
		revEnd.kind = svn_opt_revision_number;
	}
	revEnd.value.number = revisionEnd;

	//revEnd = getRevision(revisionEnd);
	preparePath(path);
	Err = svn_client_log (target ((LPCTSTR)path), 
						getRevision (revisionStart), 
						&revEnd, 
						changed,
						true,			// strict by default (not showing cp info)
						logReceiver,	// log_message_receiver
						(void *)this, &ctx, pool);

	if(Err != NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL SVN::Cat(CString url, LONG revision, CString localpath)
{
	apr_file_t * file;
	svn_stream_t * stream;
	const char *URL;

	preparePath(url);

	DeleteFile(localpath);

	apr_file_open(&file, CUnicodeUtils::GetUTF8(localpath), APR_WRITE | APR_CREATE, APR_OS_DEFAULT, pool);
	stream = svn_stream_from_aprfile(file, pool);

	Err = get_url_from_target (&URL, CUnicodeUtils::GetUTF8(url));

	if (Err != NULL)
	{
		return FALSE;
	}
	Err = svn_client_cat(stream, URL, getRevision(revision), &ctx, pool);

	apr_file_close(file);
	if (Err != NULL)
		return FALSE;
	return TRUE;
}

CString SVN::GetActionText(svn_wc_notify_action_t action, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state)
{
	CString temp = _T(" ");
	switch (action)
	{
		case svn_wc_notify_add:
		case svn_wc_notify_update_add:
		case svn_wc_notify_commit_added:
			temp.LoadString(IDS_SVNACTION_ADD);
			break;
		case svn_wc_notify_copy:
			temp.LoadString(IDS_SVNACTION_COPY);
			break;
		case svn_wc_notify_delete:
		case svn_wc_notify_update_delete:
		case svn_wc_notify_commit_deleted:
			temp.LoadString(IDS_SVNACTION_DELETE);
			break;
		case svn_wc_notify_restore:
			temp.LoadString(IDS_SVNACTION_RESTORE);
			break;
		case svn_wc_notify_revert:
			temp.LoadString(IDS_SVNACTION_REVERT);
			break;
		case svn_wc_notify_resolve:
			temp.LoadString(IDS_SVNACTION_RESOLVE);
			break;
		case svn_wc_notify_update_update:
			if ((content_state == svn_wc_notify_state_conflicted) || (prop_state == svn_wc_notify_state_conflicted))
				temp.LoadString(IDS_STATUSCONFLICTED);
			else if ((content_state == svn_wc_notify_state_merged) || (prop_state == svn_wc_notify_state_merged))
				temp.LoadString(IDS_STATUSMERGED);
			else
				temp.LoadString(IDS_SVNACTION_UPDATE);
			break;
		case svn_wc_notify_update_completed:
			temp.LoadString(IDS_SVNACTION_COMPLETED);
			break;
		case svn_wc_notify_update_external:
			temp.LoadString(IDS_SVNACTION_EXTERNAL);
			break;
		case svn_wc_notify_commit_modified:
			temp.LoadString(IDS_SVNACTION_MODIFIED);
			break;
		case svn_wc_notify_commit_replaced:
			temp.LoadString(IDS_SVNACTION_REPLACED);
			break;
		case svn_wc_notify_commit_postfix_txdelta:
			temp.LoadString(IDS_SVNACTION_POSTFIX);
			break;
		case svn_wc_notify_failed_revert:
			temp.LoadString(IDS_SVNACTION_FAILEDREVERT);
			break;
		case svn_wc_notify_status:
			temp.LoadString(IDS_SVNACTION_STATUS);
			break;
		case svn_wc_notify_skip:
			temp.LoadString(IDS_SVNACTION_SKIP);
			break;
		default:
			return _T("???");
	}
	return temp;
}

BOOL SVN::CreateRepository(CString path)
{
	apr_pool_t * localpool;
	svn_repos_t * repo;
	svn_error_t * err;
	apr_initialize();
	localpool = svn_pool_create (NULL);

	err = svn_repos_create(&repo, CUnicodeUtils::GetUTF8(path), NULL, NULL, NULL, NULL, localpool);
	if (err != NULL)
	{
		svn_pool_destroy(localpool);
		return FALSE;
	}
	svn_pool_destroy(localpool);
	apr_terminate();
	return TRUE;
}



svn_error_t* SVN::logReceiver(void* baton, 
								apr_hash_t* ch_paths, 
								svn_revnum_t rev, 
								const char* author, 
								const char* date, 
								const char* msg, 
								apr_pool_t* pool)
{
	svn_error_t * error = NULL;
	const char * date_native;
	stdstring author_native;
	stdstring msg_native;

	SVN * svn = (SVN *)baton;
	author_native = CUnicodeUtils::GetUnicode(author);

	if (date && date[0])
	{
		//Convert date to a format for humans.
		apr_time_t time_temp;

		error = svn_time_from_cstring (&time_temp, date, pool);
		if (error)
			return error;
		date_native = svn_time_to_human_cstring(time_temp, pool);
	}
	else
		date_native = "(no date)";

	if (msg == NULL)
		msg = "";

	msg_native = CUnicodeUtils::GetUnicode(msg);
	CString cpaths;

	if (ch_paths)
	{
		apr_array_header_t *sorted_paths;
		sorted_paths = apr_hash_sorted_keys(ch_paths, svn_sort_compare_items_as_paths, pool);
		for (int i = 0; i < sorted_paths->nelts; i++)
		{
			svn_item_t *item = &(APR_ARRAY_IDX (sorted_paths, i, svn_item_t));
			stdstring path_native;
			const char *path = (const char *)item->key;
			svn_log_changed_path_t *log_item = (svn_log_changed_path_t *)apr_hash_get (ch_paths, item->key, item->klen);
			const char *copy_data = "";

			if (log_item->copyfrom_path && SVN_IS_VALID_REVNUM (log_item->copyfrom_rev))
			{
				path_native = CUnicodeUtils::GetUnicode(log_item->copyfrom_path);
			} // if (log_item->copyfrom_path && SVN_IS_VALID_REVNUM (log_item->copyfrom_rev))
			else
				path_native = CUnicodeUtils::GetUnicode(path);
			CString temp;
			switch (log_item->action)
			{
			case 'M':
				temp.LoadString(IDS_SVNACTION_MODIFIED);
				break;
			case 'R':
				temp.LoadString(IDS_SVNACTION_REPLACED);
				break;
			case 'A':
				temp.LoadString(IDS_SVNACTION_ADD);
				break;
			case 'D':
				temp.LoadString(IDS_SVNACTION_DELETE);
			default:
				break;
			} // switch (temppath->action)
			if (!cpaths.IsEmpty())
				cpaths += _T("\r\n");
			cpaths += temp;
			cpaths += _T(" ");
			cpaths += path_native.c_str();
		}
	} // if (changed_paths)

	SVN_ERR (svn->cancel(baton));

	if (svn->Log(rev, CString(author_native.c_str()), CString(date_native), CString(msg_native.c_str()), cpaths))
	{
		return error;
	}
	return error;
}

void SVN::notify( void *baton,
					const char *path,
					svn_wc_notify_action_t action,
					svn_node_kind_t kind,
					const char *mime_type,
					svn_wc_notify_state_t content_state,
					svn_wc_notify_state_t prop_state,
					svn_revnum_t revision)
{
	SVN * svn = (SVN *)baton;
	WCHAR buf[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, buf, MAX_PATH);
	svn->Notify(CString(buf), (svn_wc_notify_action_t)action, kind, CString(mime_type), content_state, prop_state, revision);
}

svn_error_t* SVN::prompt(char **info, const char *prompt, svn_boolean_t hide, void *baton, apr_pool_t *pool)
{
	SVN * svn = (SVN *)baton;
	CString infostring;
	if (svn->Prompt(infostring, CString(prompt), hide))
	{
		SVN_ERR (svn_utf_cstring_to_utf8 ((const char **)info, CUnicodeUtils::GetUTF8(infostring), NULL, pool));
		return SVN_NO_ERROR;
	}
	CStringA temp;
	temp.LoadString(IDS_SVN_USERCANCELLED);
	return svn_error_create(SVN_ERR_CANCELLED, NULL, temp);
}

svn_error_t* SVN::cancel(void *baton)
{
	SVN * svn = (SVN *)baton;
	if (svn->Cancel())
	{
		CStringA temp;
		temp.LoadString(IDS_SVN_USERCANCELLED);
		return svn_error_create(SVN_ERR_CANCELLED, NULL, temp);
	}
	return SVN_NO_ERROR;
}

void * SVN::logMessage (const char * message, char * baseDirectory)
{
	log_msg_baton* baton = (log_msg_baton *) apr_palloc (pool, sizeof (*baton));
	baton->message = apr_pstrdup(pool, message);
	baton->base_dir = baseDirectory ? baseDirectory : "";

	baton->message_encoding = NULL;
	baton->tmpfile_left = NULL;
	baton->pool = pool;

	return baton;
}

svn_opt_revision_t*	SVN::getRevision (long revNumber)
{
	memset (&rev, 0, sizeof (rev));
	if(revNumber == -1)
	{
		rev.kind = svn_opt_revision_head;
		revNumber = 0;
	}
	else
	{
		rev.kind = svn_opt_revision_number;
	}

	rev.value.number = revNumber;

	return &rev;
}

void SVN::PathToUrl(CString &path)
{
	path.Trim();
	// convert \ to /
	path.Replace('\\','/');
	// prepend file:///
	path.Insert(0, _T("file:///"));
	path.TrimRight(_T("/\\"));			//remove trailing slashes
}

void SVN::UrlToPath(CString &url)
{
	url.Trim();
	url = url.Mid(8);
	url.Replace('/','\\');
	url.TrimRight(_T("/\\"));			//remove trailing slashes
}

void	SVN::preparePath(CString &path)
{
	path.Trim();
	path.TrimRight(_T("/\\"));			//remove trailing slashes
	path.Replace('\\','/');
}

svn_error_t* svn_cl__get_log_message (const char **log_msg,
									const char **tmp_file,
									apr_array_header_t * commit_items,
									void *baton, 
									apr_pool_t * pool)
{
	log_msg_baton *lmb = (log_msg_baton *) baton;
	*tmp_file = NULL;
	if (lmb->message)
	{
		*log_msg = apr_pstrdup (pool, lmb->message);
	}

	return SVN_NO_ERROR;
}

apr_array_header_t * SVN::target (LPCTSTR path)
{
	CString p = CString(path);
	apr_array_header_t *targets = apr_array_make (pool,
												5,
												sizeof (const char *));

	int curPos= 0;

	CString resToken= p.Tokenize(_T(";"),curPos);
	while (resToken != _T(""))
	{
		const char * target = apr_pstrdup (pool, CUnicodeUtils::GetUTF8(resToken));
		(*((const char **) apr_array_push (targets))) = target;
		resToken= p.Tokenize(_T(";"),curPos);
	};
	return targets;
}

svn_error_t * SVN::get_url_from_target (const char **URL, const char *target)
{
	svn_wc_adm_access_t *adm_access;          
	const svn_wc_entry_t *entry;  
	svn_boolean_t is_url = svn_path_is_url (target);

	if (is_url)
		*URL = target;

	else
	{
		SVN_ERR (svn_wc_adm_probe_open (&adm_access, NULL, target,
			FALSE, FALSE, pool));
		SVN_ERR (svn_wc_entry (&entry, target, adm_access, FALSE, pool));
		SVN_ERR (svn_wc_adm_close (adm_access));

		*URL = entry ? entry->url : NULL;
	}

	return SVN_NO_ERROR;
}

BOOL SVN::Ls(CString url, LONG revision, CStringArray& entries)
{
	entries.RemoveAll();

	apr_hash_t* hash = apr_hash_make(pool);

	Err = svn_client_ls(&hash, 
						CUnicodeUtils::GetUTF8(url),
						getRevision(revision),
						FALSE, 
						&ctx,
						pool);
	if (Err != NULL)
		return FALSE;

	apr_hash_index_t *hi;
    svn_dirent_t* val;
	const char* key;
    for (hi = apr_hash_first(pool, hash); hi; hi = apr_hash_next(hi)) 
	{
        apr_hash_this(hi, (const void**)&key, NULL, (void**)&val);
		CString temp;
		if (val->kind == svn_node_dir)
			temp = "d";
		else if (val->kind == svn_node_file)
			temp = "f";
		else
			temp = "u";
		temp = temp + CUnicodeUtils::GetUnicode(key);
		entries.Add(temp);
    }
	return Err == NULL;
}

BOOL SVN::IsRepository(const CString& strUrl)
{
	svn_repos_t* pRepos;

	Err = svn_repos_open (&pRepos, CUnicodeUtils::GetUTF8(strUrl), pool);

	return Err == NULL;
}
CString SVN::GetPristinePath(CString wcPath)
{
	apr_pool_t * localpool;
	svn_error_t * err;
	apr_initialize();
	localpool = svn_pool_create (NULL);
	const char* pristinePath = NULL;
	CString temp;

	err = svn_wc_get_pristine_copy_path(svn_path_internal_style(CUnicodeUtils::GetUTF8(wcPath), localpool), (const char **)&pristinePath, localpool);

	if (err != NULL)
	{
		svn_pool_destroy(localpool);
		return temp;
	}
	if (pristinePath != NULL)
		temp = CString(pristinePath);
	svn_pool_destroy(localpool);
	apr_terminate();
	return temp;
}
