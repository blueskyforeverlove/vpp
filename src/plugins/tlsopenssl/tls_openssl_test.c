/*
 * tls_openssl_test.c - skeleton vpp-api-test plug-in
 *
 * Copyright (c) 2019 Intel Corporation
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <vat/vat.h>
#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vppinfra/error.h>
#include <ctype.h>

uword unformat_sw_if_index (unformat_input_t * input, va_list * args);

/* Declare message IDs */
#include <tlsopenssl/tls_openssl_msg_enum.h>

/* define message structures */
#define vl_typedefs
#include <tlsopenssl/tls_openssl_all_api_h.h>
#undef vl_typedefs

/* declare message handlers for each api */

#define vl_endianfun		/* define message structures */
#include <tlsopenssl/tls_openssl_all_api_h.h>
#undef vl_endianfun

/* instantiate all the print functions we know about */
#define vl_print(handle, ...)
#define vl_printfun
#include <tlsopenssl/tls_openssl_all_api_h.h>
#undef vl_printfun

/* Get the API version number. */
#define vl_api_version(n,v) static u32 api_version=(v);
#include <tlsopenssl/tls_openssl_all_api_h.h>
#undef vl_api_version

typedef struct
{
  /* API message ID base */
  u16 msg_id_base;
  vat_main_t *vat_main;
} tls_openssl_test_main_t;

tls_openssl_test_main_t tls_openssl_test_main;

#define __plugin_msg_base tls_openssl_test_main.msg_id_base
#include <vlibapi/vat_helper_macros.h>

#define foreach_standard_reply_retval_handler   \
_(tls_openssl_set_engine_reply)

#define _(n)                                            \
    static void vl_api_##n##_t_handler                  \
    (vl_api_##n##_t * mp)                               \
    {                                                   \
        vat_main_t * vam = tls_openssl_test_main.vat_main;   \
        i32 retval = ntohl(mp->retval);                 \
        if (vam->async_mode) {                          \
            vam->async_errors += (retval < 0);          \
        } else {                                        \
            vam->retval = retval;                       \
            vam->result_ready = 1;                      \
        }                                               \
    }
foreach_standard_reply_retval_handler;
#undef _

/*
 * Table of message reply handlers, must include boilerplate handlers
 * we just generated
 */
#define foreach_vpe_api_reply_msg                       \
_(TLS_OPENSSL_SET_ENGINE_REPLY, tls_openssl_set_engine_reply)


static int
api_tls_openssl_set_engine (vat_main_t * vam)
{
  unformat_input_t *line_input = vam->input;
  vl_api_tls_openssl_set_engine_t *mp;
  u8 *engine_name = 0;
  u8 *engine_alg = 0;
  u8 *ciphers = 0;
  u32 async = 0;
  int ret;

  /* Parse args required to build the message */
  while (unformat_check_input (line_input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (line_input, "engine %s", &engine_name))
	;
      else if (unformat (line_input, "async"))
	{
	  async = 1;
	}
      else if (unformat (line_input, "alg %s", &engine_alg))
	;
      else if (unformat (line_input, "ciphers %s", &ciphers))
	;
      else
	{
	  errmsg ("unknown input `%U'", format_unformat_error, line_input);
	  return -99;
	}
    }

  if (engine_name == 0)
    {
      errmsg ("Must specify engine name");
      return -99;
    }

  if (engine_alg == 0)
    engine_alg = format (0, "ALL");
  else
    {
      for (int i = 0; i < strnlen ((char *) engine_alg, 64); i++)
	engine_alg[i] = toupper (engine_alg[i]);
    }


  /* Construct the API message */
  M (TLS_OPENSSL_SET_ENGINE, mp);
  mp->async = async;

  clib_memcpy_fast (mp->engine, engine_name,
		    strnlen ((const char *) engine_name, 64));

  clib_memcpy_fast (mp->algorithm, engine_alg,
		    strnlen ((const char *) engine_alg, 64));

  if (ciphers)
    clib_memcpy_fast (mp->ciphers, ciphers,
		      strnlen ((const char *) ciphers, 64));

  /* send it... */
  S (mp);

  /* Wait for a reply... */
  W (ret);
  return ret;
}

/*
 * List of messages that the api test plugin sends,
 * and that the data plane plugin processes
 */
#define foreach_vpe_api_msg                                      \
_(tls_openssl_set_engine, "tls openssl set [engine <engine name>]" \
"[alg [algorithm] [async]\n")

static void
tls_openssl_api_hookup (vat_main_t * vam)
{
  tls_openssl_test_main_t *htmp = &tls_openssl_test_main;
  /* Hook up handlers for replies from the data plane plug-in */
#define _(N,n)                                                  \
    vl_msg_api_set_handlers((VL_API_##N + htmp->msg_id_base),     \
                           #n,                                  \
                           vl_api_##n##_t_handler,              \
                           vl_noop_handler,                     \
                           vl_api_##n##_t_endian,               \
                           vl_api_##n##_t_print,                \
                           sizeof(vl_api_##n##_t), 1);
  foreach_vpe_api_reply_msg;
#undef _

  /* API messages we can send */
#define _(n,h) hash_set_mem (vam->function_by_name, #n, api_##n);
  foreach_vpe_api_msg;
#undef _

  /* Help strings */
#define _(n,h) hash_set_mem (vam->help_by_name, #n, h);
  foreach_vpe_api_msg;
#undef _
}

VAT_PLUGIN_REGISTER (tls_openssl);

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
