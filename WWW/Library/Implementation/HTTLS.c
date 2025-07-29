/*
 * $LynxId: HTTLS.c,v 1.21 2025/07/25 19:34:13 tom Exp $
 *
 *                          TRANSPORT LAYER SECURITY
 *
 * This file provides some utilities to create a reasonably simple TLS API,
 * without having to manage all of the different versions and forks of libssl.
 * */

#include <HTTLS.h>
#include <HTAlert.h>
#include <HTParse.h>

#include <LYUtils.h>
#include <LYGlobalDefs.h>

#ifdef USE_SSL

#ifdef USE_OPENSSL_INCL
#include <openssl/x509v3.h>
#endif

#ifdef USE_GNUTLS_INCL
#include <gnutls/x509.h>
#endif

#if defined(LIBRESSL_VERSION_NUMBER)
/* OpenSSL and LibreSSL version numbers do not correspond */

#if LIBRESSL_VERSION_NUMBER >= 0x2060100fL
#define SSL_set_no_TLSV1()		SSL_set_min_proto_version(handle, TLS1_1_VERSION)
#endif

#elif defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER >= 0x10100000L)

#define SSLEAY_VERSION_NUMBER		OPENSSL_VERSION_NUMBER
#undef  SSL_load_error_strings
#undef  SSLeay_add_ssl_algorithms
#define ASN1_STRING_data		ASN1_STRING_get0_data
#define TLS_client_method()		SSLv23_client_method()
#define SSL_load_error_strings()	/* nothing */
#define SSLeay_add_ssl_algorithms()	/* nothing */
#define SSL_set_no_TLSV1()		SSL_set_min_proto_version(handle, TLS1_1_VERSION)

#elif defined(SSLEAY_VERSION_NUMBER)

#define TLS_client_method()		SSLv23_client_method()

#endif

#ifndef SSL_set_no_TLSV1
#define SSL_set_no_TLSV1()		SSL_set_options(handle, SSL_OP_NO_TLSv1)
#endif

#ifndef SSL_OP_NO_SSLv3
#define SSL_OP_NO_SSLv3 -1
#endif

#ifndef SSL_OP_NO_TLSv1
#define SSL_OP_NO_TLSv1 -2
#endif

#ifndef SSL_OP_NO_TLSv1_1
#define SSL_OP_NO_TLSv1_1 -3
#endif

#ifndef SSL_OP_NO_TLSv1_2
#define SSL_OP_NO_TLSv1_2 -4
#endif

#ifndef SSL_OP_NO_TLSv1_3
#define SSL_OP_NO_TLSv1_3 -5
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

SSL_CTX *ssl_ctx = NULL;	/* SSL ctx */
static int ssl_okay;

#if SSLEAY_VERSION_NUMBER < 0x101000000
/* *INDENT-OFF* */
static const struct {
    int version;

    /* The name is used to get gnutls_protocol_t values for each of these
     * protocols. It's kind of ugly that we're doing a runtime string search
     * on something that could easily be determined at compile time, but I
     * think that this is still cleaner than preprocessor hell.
     */
    const char *name;

    /* This is used with SSL_set_options */
    long opt;
} tls_versions[] = {
    { SSL3_VERSION,   "SSL3.0", SSL_OP_NO_SSLv3 },
    { TLS1_VERSION,   "TLS1.0", SSL_OP_NO_TLSv1 },
    { TLS1_1_VERSION, "TLS1.1", SSL_OP_NO_TLSv1_1 },
    { TLS1_2_VERSION, "TLS1.2", SSL_OP_NO_TLSv1_2 },
    { TLS1_3_VERSION, "TLS1.3", SSL_OP_NO_TLSv1_3 },
};
/* *INDENT-ON* */
#endif /* older libssl */

static void free_ssl_ctx(void)
{
    if (ssl_ctx != NULL)
	SSL_CTX_free(ssl_ctx);
}

static BOOL needs_limit(const char *actual)
{
    return ((int) strlen(actual) > LYcols - 7) ? TRUE : FALSE;
}

static char *limited_string(const char *source, const char *actual)
{
    int limit = ((int) strlen(source)
		 - ((int) strlen(actual) - (LYcols - 10)));
    char *temp = NULL;

    StrAllocCopy(temp, source);
    if (limit < 0)
	limit = 0;
    strcpy(temp + limit, "...");
    return temp;
}

/*
 * If the error message is too long to fit in the line, truncate that to fit
 * within the limits for prompting.
 */
static void SSL_single_prompt(char **target, const char *source)
{
    HTSprintf0(target, SSL_FORCED_PROMPT, source);
    if (needs_limit(*target)) {
	char *temp = limited_string(source, *target);

	*target = NULL;
	HTSprintf0(target, SSL_FORCED_PROMPT, temp);
	free(temp);
    }
}

static void SSL_double_prompt(char **target, const char *format, const char
			      *arg1, const char *arg2)
{
    (void) format;
    HTSprintf0(target, HT_FMT("%s%s", format), arg1, arg2);
    if (needs_limit(*target)) {
	char *parg2 = limited_string(arg2, *target);

	*target = NULL;
	HTSprintf0(target, HT_FMT("%s%s", format), arg1, parg2);
	if (needs_limit(*target)) {
	    char *parg1 = limited_string(arg1, *target);

	    *target = NULL;
	    HTSprintf0(target, HT_FMT("%s%s", format), parg1, parg2);
	    free(parg1);
	}
	free(parg2);
    }
}

static int HTSSLCallback(int preverify_ok, X509_STORE_CTX * x509_ctx GCC_UNUSED)
{
    char *msg = NULL;
    int result = 1;

#ifdef USE_X509_SUPPORT
    HTSprintf0(&msg,
	       LY_MSG("SSL callback:%s, preverify_ok=%d, ssl_okay=%d"),
	       X509_verify_cert_error_string((long) X509_STORE_CTX_get_error(x509_ctx)),
	       preverify_ok, ssl_okay);
    _HTProgress(msg);
    FREE(msg);
#endif

    if (!(preverify_ok || ssl_okay || ssl_noprompt)) {
#ifdef USE_X509_SUPPORT
	SSL_single_prompt(&msg,
			  X509_verify_cert_error_string((long)
							X509_STORE_CTX_get_error(x509_ctx)));
	if (HTForcedPrompt(ssl_noprompt, msg, NO))
	    ssl_okay = 1;
	else
	    result = 0;
#endif

	FREE(msg);
    }
    return result;
}

SSL *HTGetSSLHandle(void)
{
#ifdef USE_GNUTLS_INCL
    static char *certfile = NULL;
#endif
    static char *client_keyfile = NULL;
    static char *client_certfile = NULL;

    if (ssl_ctx == NULL) {
	/*
	 * First time only.
	 */
#if SSLEAY_VERSION_NUMBER < 0x0800
	if ((ssl_ctx = SSL_CTX_new()) != NULL) {
	    X509_set_default_verify_paths(ssl_ctx->cert);
	}
#else
	SSLeay_add_ssl_algorithms();
	if ((ssl_ctx = SSL_CTX_new(TLS_client_method())) != NULL) {
#ifdef SSL_OP_NO_SSLv2
	    SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2);
#else
	    SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);
#endif
#ifdef SSL_OP_NO_COMPRESSION
	    SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_COMPRESSION);
#endif
#ifdef SSL_MODE_AUTO_RETRY
	    SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);
#endif
#ifdef SSL_MODE_RELEASE_BUFFERS
	    SSL_CTX_set_mode(ssl_ctx, SSL_MODE_RELEASE_BUFFERS);
#endif
	    SSL_CTX_set_default_verify_paths(ssl_ctx);
	    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, HTSSLCallback);
	}
#endif /* SSLEAY_VERSION_NUMBER < 0x0800 */
#if defined(USE_PROGRAM_DIR) & !defined(USE_GNUTLS_INCL)
	if (ssl_ctx != NULL) {
	    X509_LOOKUP *lookup;

	    lookup = X509_STORE_add_lookup(ssl_ctx->cert_store,
					   X509_LOOKUP_file());
	    if (lookup != NULL) {
		char *certfile = NULL;

		HTSprintf0(&certfile, "%s\\cert.pem", program_dir);
		X509_LOOKUP_load_file(lookup, certfile, X509_FILETYPE_PEM);
		FREE(certfile);
	    }
	}
#endif
#ifdef USE_GNUTLS_INCL
	if ((certfile = LYGetEnv("SSL_CERT_FILE")) != NULL) {
	    CTRACE((tfp,
		    "HTGetSSLHandle: certfile is set to %s by SSL_CERT_FILE\n",
		    certfile));
	} else {
	    if (non_empty(SSL_cert_file)) {
		certfile = SSL_cert_file;
		CTRACE((tfp,
			"HTGetSSLHandle: certfile is set to %s by config SSL_CERT_FILE\n",
			certfile));
	    }
#if defined(USE_PROGRAM_DIR)
	    else {
		HTSprintf0(&(certfile), "%s\\cert.pem", program_dir);
		CTRACE((tfp,
			"HTGetSSLHandle: certfile is set to %s by installed directory\n", certfile));
	    }
#endif
	}
#endif
	atexit(free_ssl_ctx);
    }

    if (non_empty(SSL_client_key_file)) {
	client_keyfile = SSL_client_key_file;
	CTRACE((tfp,
		"HTGetSSLHandle: client key file is set to %s by config SSL_CLIENT_KEY_FILE\n",
		client_keyfile));
    }

    if (non_empty(SSL_client_cert_file)) {
	client_certfile = SSL_client_cert_file;
	CTRACE((tfp,
		"HTGetSSLHandle: client cert file is set to %s by config SSL_CLIENT_CERT_FILE\n",
		client_certfile));
    }
#ifdef USE_GNUTLS_INCL
    ssl_ctx->certfile = certfile;
    ssl_ctx->certfile_type = GNUTLS_X509_FMT_PEM;
    ssl_ctx->client_keyfile = client_keyfile;
    ssl_ctx->client_keyfile_type = GNUTLS_X509_FMT_PEM;
    ssl_ctx->client_certfile = client_certfile;
    ssl_ctx->client_certfile_type = GNUTLS_X509_FMT_PEM;
#elif SSLEAY_VERSION_NUMBER >= 0x0930
    if (client_certfile != NULL) {
	if (client_keyfile == NULL) {
	    client_keyfile = client_certfile;
	}
	SSL_CTX_use_certificate_chain_file(ssl_ctx, client_certfile);
	SSL_CTX_use_PrivateKey_file(ssl_ctx, client_keyfile, SSL_FILETYPE_PEM);
    }
#endif
    ssl_okay = 0;
    return (SSL_new(ssl_ctx));
}

void HTSSLInitPRNG(void)
{
#if SSLEAY_VERSION_NUMBER >= 0x00905100
    if (RAND_status() == 0) {
	char rand_file[256];
	time_t t;
	long l, seed;

#ifndef _WINDOWS
	pid_t pid;

#else
	DWORD pid;
#endif

	t = time(NULL);

#ifndef _WINDOWS
	pid = getpid();
#else
	pid = GetCurrentThreadId();
#endif

	RAND_file_name(rand_file, 256L);
	CTRACE((tfp, "HTTP: Seeding PRNG\n"));
	/* Seed as much as 1024 bytes from RAND_file_name */
	RAND_load_file(rand_file, 1024L);
	/* Seed in time (mod_ssl does this) */
	RAND_seed((unsigned char *) &t, (int) sizeof(time_t));

	/* Seed in pid (mod_ssl does this) */
	RAND_seed((unsigned char *) &pid, (int) sizeof(pid));
	/* Initialize system's random number generator */
	RAND_bytes((unsigned char *) &seed, (int) sizeof(long));

	lynx_srand((unsigned) seed);
	while (RAND_status() == 0) {
	    /* Repeatedly seed the PRNG using the system's random number generator until it has been seeded with enough data */
	    l = (long) lynx_rand();
	    RAND_seed((unsigned char *) &l, (int) sizeof(long));
	}
	/* Write a rand_file */
	RAND_write_file(rand_file);
    }
#endif /* SSLEAY_VERSION_NUMBER >= 0x00905100 */
    return;
}

static void show_cert_issuer(X509 * peer_cert GCC_UNUSED)
{
#if defined(USE_OPENSSL_INCL) || defined(USE_GNUTLS_INCL)
    char ssl_dn[1024];
    char *msg = NULL;

    X509_NAME_oneline(X509_get_issuer_name(peer_cert), ssl_dn, (int) sizeof(ssl_dn));
    HTSprintf0(&msg, LY_MSG("Certificate issued by: %s"), ssl_dn);
    _HTProgress(msg);
    FREE(msg);
#endif
}

/*
 * Remove user/password, if any, from the given host-string.
 */
static char *StripUserAuthents(char *host)
{
    char *p = strchr(host, '@');

    if (p != NULL) {
	char *q = host;

	while ((*q++ = *++p) != '\0') ;
    }
    return host;
}

/*
 * Remove IPv6 brackets (and any port-number) from the given host-string.
 */
static char *StripIpv6Brackets(char *host)
{
    int port_number;
    char *p;

    if ((p = HTParsePort(host, &port_number)) != NULL)
	*p = '\0';

    if (*host == '[') {
	p = host + strlen(host) - 1;
	if (*p == ']') {
	    *p = '\0';
	    for (p = host; (p[0] = p[1]) != '\0'; ++p) {
		;		/* EMPTY */
	    }
	}
    }
    return host;
}

/* Extracts the host name from a URL. */
static char *get_host(const char *url)
{
    char *host;

    host = HTParse(url, "", PARSE_HOST);
    host = StripIpv6Brackets(host);
    host = StripUserAuthents(host);
    return host;
}

/* Configures various options on an SSL handle */
static void configure_handle(SSL * handle, char *host,
			     int min_version, int max_version)
{
    unsigned i;
    int ret, state;
    unsigned long opts, optm, retl;

    /* Silence compiler warnings about unused variables */
    (void) i;
    (void) ret;
    (void) state;
    (void) opts;
    (void) optm;
    (void) retl;

    (void) handle;
    (void) host;
    (void) min_version;
    (void) max_version;

#if defined(USE_GNUTLS_INCL)
    /* gnutls */

    ret = SSL_set_tlsext_host_name(handle, host);
    CTRACE((tfp, "...called SSL_set_tlsext_host_name(%s) ->%d\n", host, ret));

    /* If we're in state 0, that means that we haven't hit `min_version` yet.
     * If we're in state 1, we've seen `min_version` and are waiting for
     * `max_version`
     * If we're in state 2, we've seen `max_version` and are disabling
     * everything else.
     *
     * This code is kind of gross, sorry. - Nate Choe
     * */
    state = (min_version == HTTLS_ANY_VERSION) ? 1 : 0;
    for (i = 0; i < TABLESIZE(tls_versions); ++i) {
	gnutls_protocol_t proto;
	unsigned int v;

	proto = gnutls_protocol_get_id(tls_versions[i].name);
	CTRACE((tfp, "...called gnutls_protocol_get_id(%s) ->%d\n",
		tls_versions[i].name, (int) proto));

#if defined(HAVE_GNUTLS_PROTOCOL_SET_ENABLED)
	if (state == 0) {
	    if (tls_versions[i].version == min_version) {
		state = 1;
	    } else {
		v = 0;
		goto set_version;
	    }
	}

	if (state == 2) {
	    v = 0;
	    goto set_version;
	}

	if (tls_versions[i].version == max_version)
	    state = 2;

	if (proto != GNUTLS_VERSION_UNKNOWN)
	    v = 1;

      set_version:
	if (proto != GNUTLS_VERSION_UNKNOWN) {
	    ret = gnutls_protocol_set_enabled(proto, v);
	    CTRACE((tfp,
		    "...called gnutls_protocol_set_enabled(%d, %u) ->%d\n",
		    (int) proto, v, ret));
	}
#endif
    }

#elif SSLEAY_VERSION_NUMBER >= 0x0900
    /* Set host name */
#if OPENSSL_VERSION_NUMBER >= 0x0090806fL && !defined(OPENSSL_NO_TLSEXT)
    ret = (int) SSL_set_tlsext_host_name(handle, host);
    CTRACE((tfp, "...called SSL_set_tlsext_host_name(%s) ->%d\n", host, ret));
#endif

    /* Set allowed version numbers */
#if SSLEAY_VERSION_NUMBER < 0x101000000
    /* We don't have access to SSL_set_*_proto_version, so we have to handroll
     * it. */

    /* See the GnuTLS code for the explanation of state */
    state = (min_version == HTTLS_ANY_VERSION) ? 0 : 1;
    opts = optm = 0;
    for (i = 0; i < TABLESIZE(tls_versions); ++i) {
	if (tls_versions[i].opt >= 0) {
	    optm |= (unsigned long) tls_versions[i].opt;
	}
	switch (state) {
	case 0:
	    if (tls_versions[i].version == min_version)
		state = 1;
	    else
		break;
	    /* fallthrough */
	case 1:
	    if (tls_versions[i].version == max_version)
		state = 2;
	    if (tls_versions[i].opt >= 0)
		opts |= (unsigned long) tls_versions[i].opt;
	    break;
	}
    }

#if defined(SSL_clear_options) || defined(HAVE_SSL_CLEAR_OPTIONS)
    retl = (unsigned long) SSL_clear_options(handle, optm);
    CTRACE((tfp, "...called SSL_clear_options(%ld) ->%ld\n", optm, retl));
#endif
    retl = (unsigned long) SSL_set_options(handle, opts);
    CTRACE((tfp, "...called SSL_set_options(%ld) ->%ld\n", opts, retl));

#else
    min_version = MAX(min_version, 0);
    max_version = MAX(max_version, 0);
    ret = SSL_set_min_proto_version(handle, min_version);
    CTRACE((tfp, "...called SSL_set_min_proto_version(%d) ->%d\n",
	    min_version, ret));
    ret = SSL_set_max_proto_version(handle, max_version);
    CTRACE((tfp, "...called SSL_set_max_proto_version(%d) ->%d\n",
	    max_version, ret));
#endif
#endif
}

static void print_ssl_connect_error(const char *url, int status)
{
    unsigned long SSLerror;

    CTRACE((tfp,
	    "HTTP: Unable to complete SSL handshake for '%s', SSL_connect=%d, SSL error stack dump follows\n",
	    url, status));
    SSL_load_error_strings();
    while ((SSLerror = ERR_get_error()) != 0) {
	CTRACE((tfp, "HTTP: SSL: %s\n", ERR_error_string(SSLerror, NULL)));
    }
    HTAlert("Unable to make secure connection to remote host.");
}

#ifdef USE_GNUTLS_INCL
/* Checks if a handle's certificate is signed, but doesn't check that the
 * hostname is valid. */
static BOOL cert_signed_gnutls(SSL * handle)
{
    int ret;
    unsigned int tls_status;

    gnutls_certificate_set_verify_flags(handle->gnutls_cred,
					GNUTLS_VERIFY_DO_NOT_ALLOW_SAME |
					GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
    ret = gnutls_certificate_verify_peers2(handle->gnutls_state, &tls_status);

    /* GnuTLS verify has failed print an error message and potentially ask the
     * user to trust the certificate anyways */
    if (ret < 0 || tls_status != 0) {
	int flag_continue = 1;
	char *msg = NULL;

	/* Print an error message */
#if GNUTLS_VERSION_NUMBER >= 0x030104
	int type;
	gnutls_datum_t out;

	if (ret < 0) {
	    SSL_single_prompt(&msg,
			      gettext("GnuTLS error when trying to verify certificate."));
	} else {
	    type = gnutls_certificate_type_get(handle->gnutls_state);
	    (void) gnutls_certificate_verification_status_print(tls_status,
								type, &out, 0);
	    SSL_single_prompt(&msg, (const char *) out.data);
	    gnutls_free(out.data);
	}
#else
	char *msg2;

	if (ret == 0 && tls_status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
	    msg2 = gettext("the certificate has no known issuer");
	} else if (tls_status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
	    msg2 = gettext("no issuer was found");
	} else if (tls_status & GNUTLS_CERT_SIGNER_NOT_CA) {
	    msg2 = gettext("issuer is not a CA");
	} else if (tls_status & GNUTLS_CERT_REVOKED) {
	    msg2 = gettext("the certificate has been revoked");
	} else {
	    msg2 = gettext("the certificate is not trusted");
	}
	SSL_single_prompt(&msg, msg2);
#endif

	/* At this point, we know that the certificate is invalid and have
	 * printed an error message to the user. Now, the user has the option of
	 * trusting the certificate anyways. */
	CTRACE((tfp, "HTLoadHTTP: %s\n", msg));
	if (!ssl_noprompt) {
	    if (!HTForcedPrompt(ssl_noprompt, msg, NO)) {
		flag_continue = 0;
	    }
	} else if (ssl_noprompt == FORCE_PROMPT_NO) {
	    flag_continue = 0;
	}
	FREE(msg);
	if (flag_continue == 0) {
	    return FALSE;
	}
    }
    return TRUE;
}
#endif /* USE_GNUTLS_INCL */

/* Parses TLS canonical names from a DN string generated by X509_NAME_ONELINE
 *
 * I'm pretty sure OpenSSL and the GnuTLS polyfill defined in tidy_tls.c are
 * basically equivalent, but ssl_dn generally looks something like this:
 *
 *     /CN=example.com/CN=www.example.com/C=US/L=TX/Email=alice@example.com
 *
 * We're looking for every instance of /CN=* and checking to see if any of them
 * match the hostname we're looking for.
 *
 * Return values:
 *   0 - No CN found
 *   1 - CN found but no match
 *   2 - CN found which matches
 *
 * - Nate Choe
 * */
static int parse_cns(char *ssl_dn, char *host, char **all_cns)
{
    int status_sslcertcheck;
    char *cert_host, *p, *msg;

    /*
     * X.509 DN validation taking ALL CN fields into account
     * (c) 2006 Thorsten Glaser <tg@mirbsd.de>
     */

    /* initialise status information */
    status_sslcertcheck = 0;	/* 0 = no CN found in DN */

    msg = NULL;

    /* validate all CNs found in DN */
    CTRACE((tfp, "Validating CNs in '%s'\n", ssl_dn));
    while ((cert_host = strstr(ssl_dn, "/CN=")) != NULL) {
	status_sslcertcheck = 1;	/* 1 = could not verify CN */
	/* start of CommonName */
	cert_host += 4;
	/* find next part of DistinguishedName */
	if ((p = StrChr(cert_host, '/')) != NULL) {
	    *p = '\0';
	    ssl_dn = p;		/* yes this points to the NUL byte */
	} else
	    ssl_dn = NULL;
	cert_host = StripIpv6Brackets(cert_host);

	/* verify this CN */
	CTRACE((tfp, "Matching\n\tssl_host  '%s'\n\tcert_host '%s'\n",
		host, cert_host));
	if (!strcasecomp_asterisk(host, cert_host)) {
	    status_sslcertcheck = 2;	/* 2 = verified peer */
	    /* I think this is cool to have in the logs -TG */
	    HTSprintf0(&msg,
		       LY_MSG("Verified connection to %s (cert=%s)"),
		       host, cert_host);
	    _HTProgress(msg);
	    FREE(msg);
	    /* no need to continue the verification loop */

	    /* Actually, we might want to continue filling in all_cns.
	     * - Nate Choe */
	    break;
	}

	/* add this CN to list of failed CNs */
	if (*all_cns == NULL)
	    HTSACopy(all_cns, "CN<");
	else
	    HTSACopy(all_cns, ":CN<");
	HTSACopy(all_cns, cert_host);
	HTSACopy(all_cns, ">");
	/* if we cannot retry, don't try it */
	if (ssl_dn == NULL)
	    break;
	/* now retry next CN found in DN */
	*ssl_dn = '/';		/* formerly NUL byte */
    }
    return status_sslcertcheck;
}

/* Checks X.509v3 Subject Alternative Name
 *
 * Return values are the same as parse_cns.
 * */
static int altname_valid(X509 * peer_cert, char *host, char **all_cns)
{
#if defined(USE_GNUTLS_INCL)
    unsigned int i;
    int ret;
    size_t size;
    gnutls_x509_crt_t cert;
    char buf[2048], *msg;

    /* import the certificate to the x509_crt format */
    if (gnutls_x509_crt_init(&cert) != 0) {
	return 0;
    }

    if (gnutls_x509_crt_import(cert, peer_cert, GNUTLS_X509_FMT_DER) < 0) {
	gnutls_x509_crt_deinit(cert);
	return 0;
    }

    ret = 0;
    for (i = 0; ret >= 0; ++i) {
	size = sizeof(buf);
	ret = gnutls_x509_crt_get_subject_alt_name(cert, i, buf, &size, NULL);
	if (strcasecomp_asterisk(host, buf) != 0) {
	    continue;
	}
	msg = NULL;
	HTSprintf0(&msg,
		   LY_MSG("Verified connection to %s (subj=%s)"),
		   host, buf);
	_HTProgress(msg);
	FREE(msg);
	return 2;
    }
    return 0;
#elif defined(USE_OPENSSL_INCL)
    STACK_OF(GENERAL_NAME) * gens;
    int i, numalts, result;
    const GENERAL_NAME *gn;
    char *cert_host, *msg;

    gens = (STACK_OF(GENERAL_NAME) *)
	X509_get_ext_d2i(peer_cert, NID_subject_alt_name, NULL, NULL);
    if (gens == NULL) {
	return FALSE;
    }

    result = 0;

    numalts = sk_GENERAL_NAME_num(gens);
    for (i = 0; i < numalts; ++i) {
	gn = sk_GENERAL_NAME_value(gens, i);
	if (gn->type == GEN_DNS) {
	    cert_host = (char *) ASN1_STRING_data(gn->d.ia5);
	} else if (gn->type == GEN_IPADD) {
	    /* XXX untested -TG */
	    size_t j = (size_t) ASN1_STRING_length(gn->d.ia5);

	    cert_host = (char *) malloc(j + 1);
	    MemCpy(cert_host, ASN1_STRING_data(gn->d.ia5), j);
	    cert_host[j] = '\0';
	} else {
	    continue;
	}
	result = 1;		/* got at least one */
	/* verify this SubjectAltName (see parse_cns) */
	cert_host = StripIpv6Brackets(cert_host);
	if (!(gn->type == GEN_IPADD ? strcasecomp :
	      strcasecomp_asterisk) (host, cert_host)) {
	    result = 2;
	    msg = NULL;
	    HTSprintf0(&msg,
		       LY_MSG("Verified connection to %s (subj=%s)"),
		       host, cert_host);
	    _HTProgress(msg);
	    FREE(msg);
	    if (gn->type == GEN_IPADD)
		free(cert_host);
	    break;
	}
	/* add to list of failed CNs */
	if (*all_cns == NULL)
	    HTSACopy(all_cns, "SAN<");
	else
	    HTSACopy(all_cns, ":SAN<");
	if (gn->type == GEN_DNS)
	    HTSACopy(all_cns, "DNS=");
	else if (gn->type == GEN_IPADD)
	    HTSACopy(all_cns, "IP=");
	HTSACopy(all_cns, cert_host);
	HTSACopy(all_cns, ">");
	if (gn->type == GEN_IPADD)
	    free(cert_host);
    }
    sk_GENERAL_NAME_free(gens);

    return result;
#else
    return 0;
#endif

    /* Silence compiler warnings about unused arguments */
    (void) peer_cert;
    (void) host;
    (void) all_cns;
}

static BOOL cert_valid(SSL * handle, char *host)
{
    X509 *peer_cert;		/* The peer certificate */
    char ssl_dn[1024], *msg;
    char *all_cns = NULL;
    int status;

    /* OpenSSL certificate verification is handled by HTSSLCallback. */
#ifdef USE_GNUTLS_INCL
    if (!cert_signed_gnutls(handle)) {
	return FALSE;
    }
#endif

    peer_cert = (X509 *) SSL_get_peer_certificate(handle);
    X509_NAME_oneline(X509_get_subject_name(peer_cert),
		      ssl_dn, (int) sizeof(ssl_dn));

    status = parse_cns(ssl_dn, host, &all_cns);
    if (status < 2) {
	int s2;

	s2 = altname_valid(peer_cert, host, &all_cns);
	status = MAX(status, s2);
    }

    msg = NULL;
    switch (status) {
    case 0:
	SSL_single_prompt(&msg,
			  gettext("Can't find common name in certificate"));
	break;
    case 1:
	SSL_double_prompt(&msg,
			  LY_MSG("SSL error:host(%s)!=cert(%s)-Continue?"),
			  host, all_cns);
	break;
    case 2:
	goto success;
    default:
	msg = NULL;
	break;
    }

    /* If we reach this point then we've failed to notify the certificate, we
     * notified the user, and we're asking if we should trust this
     * certificate anyways. */

    if (msg == NULL)
	StrAllocCopy(msg, gettext("SSL error"));
    if (!HTForcedPrompt(ssl_noprompt, msg, NO)) {
	FREE(msg);
	FREE(all_cns);
	return FALSE;
    }

    SSL_double_prompt(&msg,
		      LY_MSG("UNVERIFIED connection to %s (cert=%s)"),
		      host, all_cns ? all_cns : "NONE");
    _HTProgress(msg);
    FREE(msg);
  success:
    show_cert_issuer(peer_cert);
    FREE(all_cns);
    return TRUE;
}

SSL *HTSSLConnect(int fd, const char *url, int min_version, int max_version)
{
    SSL *handle;
    char *host;
    int status;

    handle = HTGetSSLHandle();
    SSL_set_fd(handle, fd);
    host = get_host(url);
    configure_handle(handle, host, min_version, max_version);
    HTSSLInitPRNG();
    status = SSL_connect(handle);
    if (status <= 0) {
	print_ssl_connect_error(url, status);
	goto error;
    }
    if (!cert_valid(handle, host)) {
	goto error;
    }

    return handle;
  error:
    SSL_free(handle);
    return NULL;
}

#endif /* USE_SSL */
