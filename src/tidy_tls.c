/*
 * $LynxId: tidy_tls.c,v 1.41 2020/03/03 11:46:07 Gisle.Vanem Exp $
 * Copyright 2008-2019,2020 Thomas E. Dickey
 * with fix Copyright 2008 by Thomas Viehmann
 *
 * Required libraries:
 *	libgnutls
 *	libcrypt
 */
#include <HTUtils.h>
#include <tidy_tls.h>

#include <gnutls/x509.h>
#ifdef HAVE_GNUTLS_RND
#include <gnutls/crypto.h>
#else
#include <gcrypt.h>
#endif
#include <libtasn1.h>		/* ASN1_SUCCESS,etc */
#include <string.h>

#define typeCalloc(type) (type *) calloc(1, sizeof(type))

static int last_error = 0;

/* ugly, but hey, we could just use a more sane api, too */
#define GetDnByOID(target, oid, thewhat) \
	len = sizeof(target); \
	if (! thewhat) \
	    gnutls_x509_crt_get_dn_by_oid(xcert, oid, 0, 0, target, &len); \
	else \
	    gnutls_x509_crt_get_issuer_dn_by_oid(xcert, oid, 0, 0, target, &len)

/* thewhat: which DN to get 0 = subject, 1 = issuer */
static int ExtractCertificate(const gnutls_datum_t *cert, X509_NAME * result, int thewhat)
{
    gnutls_x509_crt_t xcert;
    int rc;
    size_t len;

    if ((rc = gnutls_x509_crt_init(&xcert)) >= 0) {
	if ((rc = gnutls_x509_crt_import(xcert, cert, GNUTLS_X509_FMT_DER)) >= 0) {
	    GetDnByOID(result->country,
		       GNUTLS_OID_X520_COUNTRY_NAME, thewhat);
	    GetDnByOID(result->organization,
		       GNUTLS_OID_X520_ORGANIZATION_NAME, thewhat);
	    GetDnByOID(result->organizational_unit_name,
		       GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME, thewhat);
	    GetDnByOID(result->common_name,
		       GNUTLS_OID_X520_COMMON_NAME, thewhat);
	    GetDnByOID(result->locality_name,
		       GNUTLS_OID_X520_LOCALITY_NAME, thewhat);
	    GetDnByOID(result->state_or_province_name,
		       GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME, thewhat);
	    GetDnByOID(result->email,
		       GNUTLS_OID_PKCS9_EMAIL, thewhat);
	    rc = 0;
	}
	gnutls_x509_crt_deinit(xcert);
    }
    return rc;
}

/*
 * (stub)
 * ERR_error_string() generates a human-readable string representing the
 * error code 'e', and places it at 'buffer', which must be at least 120 bytes
 * long. If 'buffer' is NULL, the error string is placed in a static buffer.
 */
const char *ERR_error_string(unsigned long e, char *buffer)
{
    (void) buffer;
    return gnutls_strerror((int) -e);
}

/*
 * (stub)
 * Return the earliest error code from the thread's error queue and remove the
 * entry.
 */
unsigned long ERR_get_error(void)
{
    unsigned long rc;

    rc = (unsigned long) (-last_error);
    last_error = 0;

    return rc;
}

/*
 * Put 'num' cryptographically strong pseudo-random bytes into 'buffer'.
 */
int RAND_bytes(unsigned char *buffer, int num)
{
    int rc;

#ifdef HAVE_GNUTLS_RND
    rc = gnutls_rnd(GNUTLS_RND_KEY, buffer, (size_t) num);
#else
    gcry_randomize(buffer, num, GCRY_VERY_STRONG_RANDOM);
    rc = 1;
#endif
    return rc;
}

/*
 * (stub)
 * Generate a default path for the random seed file.  'buffer' points to a
 * buffer of size 'len' in which to store the filename.
 */
const char *RAND_file_name(char *buffer, size_t len)
{
    (void) buffer;
    (void) len;
    return "";
}

/*
 * (stub)
 * Read a number of bytes from file 'name' and adds them to the PRNG.  If
 * 'maxbytes' is non-negative, up to to 'maxbytes' are read; if 'maxbytes' is
 * -1, the complete file is read.
 */
int RAND_load_file(const char *name, long maxbytes)
{
    (void) name;
    return (int) maxbytes;
}

/*
 * (stub)
 * Mix the 'num' bytes at 'buffer' into the PRNG state.
 */
void RAND_seed(const void *buffer, int num)
{
    (void) buffer;
    (void) num;
}

/*
 * (stub)
 * Return 1 if the PRNG has been seeded with enough data, 0 otherwise.
 */
int RAND_status(void)
{
    return 1;
}

/*
 * (stub)
 * Write a number of random bytes (currently 1024) to file 'name' which can be
 * used to initialize the PRNG by calling RAND_load_file() in a later session.
 */
int RAND_write_file(const char *name)
{
    (void) name;
    return 0;
}

/*
 * Return the number of secret bits used for cipher.  If 'bits' is not NULL, it
 * contains the number of bits processed by the chosen algorithm.  If cipher is
 * NULL, 0 is returned.
 */
int SSL_CIPHER_get_bits(SSL_CIPHER * cipher, int *bits)
{
    int result = 0;

    if (cipher) {
	result = (8 * (int) gnutls_cipher_get_key_size(cipher->encrypts));
    }

    if (bits)
	*bits = result;

    return result;
}

/*
 * Return a pointer to the name of 'cipher'.  If 'cipher' is NULL the constant
 * value "NONE" is returned.
 */
const char *SSL_CIPHER_get_name(SSL_CIPHER * cipher)
{
    const char *result = "NONE";

    if (cipher) {
	result = gnutls_cipher_suite_get_name(cipher->key_xchg,
					      cipher->encrypts,
					      cipher->msg_code);
    }
    return result;
}

/*
 * Return the protocol version for cipher, currently "SSLv2", "SSLv3", or
 * "TLSv1".  If cipher is NULL, "(NONE)" is returned.
 */
const char *SSL_CIPHER_get_version(SSL_CIPHER * cipher)
{
    const char *result = "NONE";

    if (cipher) {
	if ((result = gnutls_protocol_get_name(cipher->protocol)) == 0)
	    result = "unknown";
    }
    return result;
}

/*
 * Free an allocated SSL_CTX object.
 */
void SSL_CTX_free(SSL_CTX * ctx)
{
    free(ctx->method);
    free(ctx);
}

/*
 * Create a new SSL_CTX object as framework for TLS/SSL enabled functions.
 */
SSL_CTX *SSL_CTX_new(SSL_METHOD * method)
{
    SSL_CTX *ctx;

    if ((ctx = typeCalloc(SSL_CTX)) != 0) {
	ctx->method = method;
    }

    return ctx;
}

/*
 * See SSL_CTX_load_verify_locations() - this sets the paths for that and
 * SSL_CTX_set_verify() to their default values.  GNU TLS does not have a
 * comparable feature (stub).
 */
int SSL_CTX_set_default_verify_paths(SSL_CTX * ctx)
{
    (void) ctx;
    return 0;
}

/*
 * SSL_CTX_set_options() adds the options set via bitmask in options to
 * ctx.  Options already set before are not cleared.
 */
unsigned long SSL_CTX_set_options(SSL_CTX * ctx, unsigned long options)
{
    ctx->options |= options;
    return ctx->options;
}

/*
 * Set peer certificate verification parameters.
 */
void SSL_CTX_set_verify(SSL_CTX * ctx, int verify_mode,
			int (*verify_callback) (int, X509_STORE_CTX *))
{
    ctx->verify_mode = verify_mode;
    ctx->verify_callback = verify_callback;
}

#ifdef HAVE_GNUTLS_PROTOCOL_SET_PRIORITY
static void RemoveProtocol(SSL * ssl, int protocol)
{
    int j, k;
    int changed = 0;
    int *protocols = ssl->ctx->method->priority.protocol;

    for (j = k = 0; j < GNUTLS_MAX_ALGORITHM_NUM;) {
	if (protocols[k] == protocol) {
	    if (++k >= GNUTLS_MAX_ALGORITHM_NUM)
		break;
	    changed = 1;
	} else {
	    protocols[j++] = protocols[k++];
	}
    }

    if (changed) {
	gnutls_protocol_set_priority(ssl->gnutls_state, protocols);
    }
}
#endif

/*
 * Initiate the TLS/SSL handshake with an TLS/SSL server.
 */
int SSL_connect(SSL * ssl)
{
    X509_STORE_CTX *store;
    int rc;
    gnutls_alert_description_t alert;
    const char *aname;

    if (ssl->options & SSL_OP_NO_TLSv1) {
#ifdef HAVE_GNUTLS_PROTOCOL_SET_PRIORITY
	RemoveProtocol(ssl, GNUTLS_TLS1);
#else
	gnutls_priority_set_direct(ssl->gnutls_state, "NORMAL:-VERS-TLS1.0", NULL);
#endif
    }

    while ((rc = gnutls_handshake(ssl->gnutls_state)) < 0 &&
	   !gnutls_error_is_fatal(rc)) {
	if (rc == GNUTLS_E_WARNING_ALERT_RECEIVED) {
	    alert = gnutls_alert_get(ssl->gnutls_state);
	    aname = gnutls_alert_get_name(alert);
	    CTRACE((tfp, "SSL Alert: %s\n", NonNull(aname)));
	    switch (gnutls_alert_get(ssl->gnutls_state)) {
	    case GNUTLS_A_UNRECOGNIZED_NAME:
		continue;	/* ignore */
	    default:
		break;
	    }
	    break;		/* treat all other alerts as fatal */
	}
    }
    ssl->last_error = rc;

    if (rc < 0) {
	last_error = rc;
	return 0;
    }

    store = typeCalloc(X509_STORE_CTX);
    if (store == 0)
	outofmem(__FILE__, "SSL_connect");

    store->ssl = ssl;
    store->cert_list = SSL_get_peer_certificate(ssl);

    if (ssl->verify_callback) {
	ssl->verify_callback(1, store);
    }
    ssl->state = SSL_ST_OK;

    free(store);

    /* FIXME: deal with error from callback */

    return 1;
}

/*
 * Free an allocated SSL structure.
 */
void SSL_free(SSL * ssl)
{
    gnutls_certificate_free_credentials(ssl->gnutls_cred);
    gnutls_deinit(ssl->gnutls_state);
    free(ssl);
}

/*
 * Get SSL_CIPHER data of a connection.
 */
SSL_CIPHER *SSL_get_current_cipher(SSL * ssl)
{
    SSL_CIPHER *result = 0;

    if (ssl) {
	result = &(ssl->ciphersuite);

	result->protocol = gnutls_protocol_get_version(ssl->gnutls_state);
	result->encrypts = gnutls_cipher_get(ssl->gnutls_state);
	result->key_xchg = gnutls_kx_get(ssl->gnutls_state);
	result->msg_code = gnutls_mac_get(ssl->gnutls_state);
	result->cert = gnutls_certificate_type_get(ssl->gnutls_state);
#if !defined(_GNUTLS_GCC_VERSION) || (_GNUTLS_GCC_VERSION < 30100)
	result->compress = gnutls_compression_get(ssl->gnutls_state);
#else
	result->compress = GNUTLS_COMP_UNKNOWN;
#endif
    }

    return result;
}

/*
 * Get the X509 certificate of the peer.
 */
const X509 *SSL_get_peer_certificate(SSL * ssl)
{
    const gnutls_datum_t *result;
    unsigned list_size = 0;

    result =
	(const gnutls_datum_t *) gnutls_certificate_get_peers(ssl->gnutls_state,
							      &list_size);

    return (const X509 *) result;
}

/*
 * Initialize SSL library by registering algorithms.
 */
int SSL_library_init(void)
{
    gnutls_global_init();
    return 1;
}

/*
 * (stub)
 * OpenSSL uses this to prepare for ERR_get_error() calls.
 */
void SSL_load_error_strings(void)
{
}

/*
 * Create a new SSL structure for a connection.
 */
SSL *SSL_new(SSL_CTX * ctx)
{
    SSL *ssl;
    int rc;

    if ((ssl = typeCalloc(SSL)) != 0) {

	rc = gnutls_certificate_allocate_credentials(&ssl->gnutls_cred);
	if (rc < 0) {
	    last_error = rc;
	    free(ssl);
	    ssl = 0;
	} else {
	    ssl->ctx = ctx;

	    gnutls_init(&ssl->gnutls_state, ctx->method->connend);
	    gnutls_set_default_priority(ssl->gnutls_state);

	    gnutls_credentials_set(ssl->gnutls_state, GNUTLS_CRD_CERTIFICATE,
				   ssl->gnutls_cred);
	    if (ctx->certfile)
		gnutls_certificate_set_x509_trust_file(ssl->gnutls_cred,
						       ctx->certfile,
						       ctx->certfile_type);
	    if (ctx->client_keyfile)
		gnutls_certificate_set_x509_key_file(ssl->gnutls_cred,
						     ctx->client_certfile,
						     ctx->client_keyfile,
						     ctx->client_keyfile_type);
	    ssl->verify_mode = ctx->verify_mode;
	    ssl->verify_callback = ctx->verify_callback;

	    ssl->options = ctx->options;

	    ssl->rfd = (gnutls_transport_ptr_t) (-1);
	    ssl->wfd = (gnutls_transport_ptr_t) (-1);
	    ssl->bytes_sent = 0;
	    ssl->sendbuffer = 0;
	}
    }

    return ssl;
}

/*
 * Read 'length' bytes into 'buffer' from the given SSL connection.
 * Returns the number of bytes read, or zero on error.
 */
int SSL_read(SSL * ssl, void *buffer, int length)
{
    int rc;

    do {
	rc = (int) gnutls_record_recv(ssl->gnutls_state, buffer, (size_t) length);
    }
    while ((rc == GNUTLS_E_AGAIN) || (rc == GNUTLS_E_INTERRUPTED));

    if (rc < 0 && gnutls_error_is_fatal(rc) == 0) {
	if (rc == GNUTLS_E_REHANDSHAKE) {
	    (void) gnutls_handshake(ssl->gnutls_state);
	    do {
		rc = (int) gnutls_record_send(ssl->gnutls_state,
					      ssl->sendbuffer,
					      (size_t) ssl->bytes_sent);
	    }
	    while ((rc == GNUTLS_E_AGAIN) || (rc == GNUTLS_E_INTERRUPTED));
	    do {
		rc = (int) gnutls_record_recv(ssl->gnutls_state,
					      buffer,
					      (size_t) length);
	    }
	    while ((rc == GNUTLS_E_AGAIN) || (rc == GNUTLS_E_INTERRUPTED));
	}
    }

    ssl->last_error = rc;

    if (rc < 0) {
	last_error = rc;
	rc = 0;
    }

    return rc;
}

#ifdef _WINDOWS
static int Lynx_gtls_push(void *s, const void *buf, size_t len)
{
    return NETWRITE((SOCKET) s, buf, len);
}

/* This calls 'recv()' in a thread for every GnuTls pull. Maybe too much overhead?
 */
static int Lynx_gtls_pull(void *s, void *buf, size_t len)
{
    return NETREAD((SOCKET) s, buf, len);
}
#endif

/*
 * Connect the SSL object with a file descriptor.
 * This always returns 1 (success) since GNU TLS does not check for errors.
 */
int SSL_set_fd(SSL * ssl, int fd)
{
#ifdef _WINDOWS
    /* register callback functions to send and receive data. */
    gnutls_transport_set_push_function(ssl->gnutls_state, Lynx_gtls_push);
    gnutls_transport_set_pull_function(ssl->gnutls_state, Lynx_gtls_pull);
#endif

    gnutls_transport_set_ptr(ssl->gnutls_state,
			     (gnutls_transport_ptr_t) (intptr_t) (fd));
    return 1;
}

/*
 * Write 'length' bytes from 'buffer' to the given SSL connection.
 */
int SSL_write(SSL * ssl, const void *buffer, int length)
{
    int rc;

    do {
	rc = (int) gnutls_record_send(ssl->gnutls_state, buffer, (size_t) length);
    }
    while ((rc == GNUTLS_E_AGAIN) || (rc == GNUTLS_E_INTERRUPTED));
    ssl->last_error = rc;

    if (rc < 0) {
	last_error = rc;
	rc = 0;
    } else {
	size_t need = (size_t) rc;

	free(ssl->sendbuffer);
	ssl->sendbuffer = malloc(need);
	ssl->bytes_sent = need;
    }

    return rc;
}

/*
 * Return method-data for SSL version 3, with the option of rollback to SSL
 * version 2.
 */
SSL_METHOD *SSLv23_client_method(void)
{
    SSL_METHOD *m;

    if ((m = typeCalloc(SSL_METHOD)) != 0) {
	int n;

	/*
	 * List the protocols in decreasing order of priority.
	 */
	n = 0;
#if GNUTLS_VERSION_NUMBER >= 0x030000
	m->priority.protocol[n++] = GNUTLS_SSL3;
	m->priority.protocol[n++] = GNUTLS_TLS1_2;
#endif
	m->priority.protocol[n++] = GNUTLS_TLS1_1;
	m->priority.protocol[n++] = GNUTLS_TLS1_0;
	m->priority.protocol[n] = 0;

	/*
	 * List the cipher algorithms in decreasing order of priority.
	 */
	n = 0;
#if GNUTLS_VERSION_NUMBER >= 0x030000
	m->priority.encrypts[n++] = GNUTLS_CIPHER_AES_256_GCM;
	m->priority.encrypts[n++] = GNUTLS_CIPHER_AES_128_GCM;
#endif
	m->priority.encrypts[n++] = GNUTLS_CIPHER_AES_256_CBC;
	m->priority.encrypts[n++] = GNUTLS_CIPHER_AES_128_CBC;
	m->priority.encrypts[n++] = GNUTLS_CIPHER_CAMELLIA_256_CBC;
	m->priority.encrypts[n++] = GNUTLS_CIPHER_CAMELLIA_128_CBC;
	m->priority.encrypts[n++] = GNUTLS_CIPHER_3DES_CBC;
	m->priority.encrypts[n] = 0;

	/*
	 * List the compression algorithms in decreasing order of priority.
	 */
	n = 0;
	m->priority.compress[n++] = GNUTLS_COMP_NULL;
	m->priority.compress[n] = 0;

	/*
	 * List the key exchange algorithms in decreasing order of priority.
	 */
	n = 0;
	m->priority.key_xchg[n++] = GNUTLS_KX_DHE_RSA;
	m->priority.key_xchg[n++] = GNUTLS_KX_RSA;
	m->priority.key_xchg[n++] = GNUTLS_KX_DHE_DSS;
	m->priority.key_xchg[n] = 0;

	/*
	 * List message authentication code (MAC) algorithms in decreasing
	 * order of priority to specify via gnutls_mac_set_priority().
	 */
	n = 0;
	m->priority.msg_code[n++] = GNUTLS_MAC_SHA1;
	m->priority.msg_code[n++] = GNUTLS_MAC_MD5;
	m->priority.msg_code[n] = 0;

	/*
	 * For gnutls_init, says we're a client.
	 */
	m->connend = GNUTLS_CLIENT;
    }

    return m;
}

static int add_name(char *target, int len, const char *tag, const char *data)
{
    if (*data != '\0') {
	int need = (int) strlen(tag) + 2;

	target += strlen(target);
	if (need < len) {
	    strcat(target, "/");
	    strcat(target, tag);
	    strcat(target, "=");
	    len -= need;
	    target += need;
	}
	need = (int) strlen(data);
	if (need >= len - 1)
	    need = len - 1;
	strncat(target, data, (size_t) need)[need] = '\0';
    }
    return len;
}
#define ADD_NAME(tag, data) len = add_name(target, len, tag, data);

/*
 * Convert the X509 name 'source' to a string in the given buffer 'target',
 * whose length is 'len'.  Return a pointer to the buffer.
 */
char *X509_NAME_oneline(X509_NAME * source, char *target, int len)
{
    if (target && (len > 0)) {
	*target = '\0';
	if (source) {
	    ADD_NAME("C", source->country);
	    ADD_NAME("ST", source->state_or_province_name);
	    ADD_NAME("L", source->locality_name);
	    ADD_NAME("O", source->organization);
	    ADD_NAME("OU", source->organizational_unit_name);
	    ADD_NAME("CN", source->common_name);
	    ADD_NAME("Email", source->email);
	}
    }
    return target;
}

/*
 * Extract the certificate's issuer-name data.
 */
X509_NAME *X509_get_issuer_name(const X509 * cert)
{
    static X509_NAME *result;

    free(result);
    if ((result = typeCalloc(X509_NAME)) != 0) {
	if (ExtractCertificate(cert, result, 1) < 0) {
	    free(result);
	    result = 0;
	}
    }
    return result;
}

/*
 * Extract the certificate's subject-name data.
 */
X509_NAME *X509_get_subject_name(const X509 * cert)
{
    static X509_NAME *result;

    free(result);
    if ((result = typeCalloc(X509_NAME)) != 0) {
	if (ExtractCertificate(cert, result, 0) < 0) {
	    free(result);
	    result = 0;
	}
    }
    return result;
}
