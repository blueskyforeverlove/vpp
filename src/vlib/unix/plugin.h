/*
 * plugin.h: plugin handling
 *
 * Copyright (c) 2011 Cisco and/or its affiliates.
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

#ifndef __included_plugin_h__
#define __included_plugin_h__

#include <vlib/vlib.h>
#include <vlib/unix/unix.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * vlib plugin scheme
 *
 * Almost anything which can be made to work in a vlib unix
 * application will also work in a vlib plugin.
 *
 * The elf-section magic which registers static objects
 * works so long as plugins are preset when the vlib unix process
 * starts. But wait: there's more...
 *
 * If an application calls vlib_load_new_plugins() -- possibly after
 * changing vlib_plugin_main.plugin_path / vlib_plugin_main.plugin_name_filter,
 * -- new plugins will be loaded. That, in turn, allows considerable
 * flexibility in terms of adding feature code or fixing bugs without
 * requiring the data-plane process to restart.
 *
 * When the plugin mechanism loads a plugin, it uses dlsym to locate
 * and call the plugin's function vlib_plugin_register() if it exists.
 * A plugin which expects to be loaded after the vlib application
 * starts uses this callback to modify the application. If vlib_plugin_register
 * returns non-zero, the plugin mechanism dlclose()'s the plugin.
 *
 * Applications control the plugin search path and name filter by
 * declaring the variables vlib_plugin_path and vlib_plugin_name_filter.
 * libvlib.la supplies weak references for these symbols which
 * effectively disable the scheme. In order for the elf-section magic to
 * work, static plugins must be loaded at the earliest possible moment.
 *
 * An application can change these parameters at any time and call
 * vlib_load_new_plugins().
 */

typedef struct
{
  CLIB_CACHE_LINE_ALIGN_MARK (cacheline0);
  u8 default_disabled : 1;
  u8 deep_bind : 1;
  const char version[64];
  const char version_required[64];
  const char overrides[256];
  const char *early_init;
  const char *description;
} vlib_plugin_registration_t;

/*
 * Plugins may also use this registration format, which is
 * easy enough to emit from e.g. a golang compiler.
 */
typedef struct
{
  uword data_segment_offset;
  uword length;
} vlib_r2_string_t;

typedef struct
{
  int default_disabled;
  vlib_r2_string_t version;
  vlib_r2_string_t version_required;
  vlib_r2_string_t overrides;
  vlib_r2_string_t early_init;
  vlib_r2_string_t description;
} vlib_plugin_r2_t;

#define foreach_r2_string_field                 \
_(version)                                      \
_(version_required)                             \
_(overrides)                                    \
_(early_init)                                   \
_(description)

typedef struct
{
  u8 *name;
  u8 *filename;
  struct stat file_info;
  void *handle;

  /* plugin registration */
  vlib_plugin_registration_t *reg;
  char *version;
} plugin_info_t;

typedef struct
{
  char *name;
  u8 is_disabled;
  u8 is_enabled;
  u8 skip_version_check;
} plugin_config_t;

typedef struct
{
  /* loaded plugin info */
  plugin_info_t *plugin_info;
  uword *plugin_by_name_hash;
  uword *plugin_overrides_by_name_hash;

  /* paths and name filters */
  u8 *plugin_path;
  u8 *plugin_path_add;
  u8 *plugin_name_filter;
  u8 *vat_plugin_path;
  u8 *vat_plugin_name_filter;
  u8 plugins_default_disable;

  /* plugin configs and hash by name */
  plugin_config_t *configs;
  uword *config_index_by_name;

  /* Plugin log, avoid filling syslog w/ junk */
  vlib_log_class_t logger;

  /* usual */
  vlib_main_t *vlib_main;
} plugin_main_t;

extern plugin_main_t vlib_plugin_main;

clib_error_t *vlib_plugin_config (vlib_main_t * vm, unformat_input_t * input);
int vlib_plugin_early_init (vlib_main_t * vm);
int vlib_load_new_plugins (plugin_main_t * pm, int from_early_init);
void *vlib_get_plugin_symbol (char *plugin_name, char *symbol_name);
u8 *vlib_get_vat_plugin_path (void);

#define VLIB_PLUGIN_REGISTER() \
  vlib_plugin_registration_t vlib_plugin_registration \
  __clib_export __clib_section(".vlib_plugin_registration")

/* Call a plugin init function: used for init function dependencies. */
#define vlib_call_plugin_init_function(vm,p,x)                  \
({                                                              \
  clib_error_t *(*_f)(vlib_main_t *);                           \
  uword *_fptr = 0;                                             \
  clib_error_t * _error = 0;                                    \
  _fptr= vlib_get_plugin_symbol                                 \
    (p, CLIB_STRING_MACRO(_vlib_init_function_##x));            \
  if (_fptr == 0)                                               \
    {                                                           \
      _error = clib_error_return                                \
        (0, "Plugin %s and/or symbol %s not found.",            \
         p, CLIB_STRING_MACRO(_vlib_init_function_##x));        \
    }                                                           \
  else                                                          \
    {                                                           \
      _f = (void *)(_fptr[0]);                                  \
    }                                                           \
  if (_fptr && ! hash_get (vm->init_functions_called, _f))      \
    {                                                           \
      hash_set1 (vm->init_functions_called, _f);                \
      _error = _f (vm);                                         \
    }                                                           \
  _error;                                                       \
 })

#endif /* __included_plugin_h__ */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
