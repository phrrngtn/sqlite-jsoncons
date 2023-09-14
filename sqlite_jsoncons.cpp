
#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <iostream>
#include <nlohmann/json.hpp>

#ifdef WIN32
#define SQLITE_EXTENSION_ENTRY_POINT __declspec(dllexport)
#else
#define SQLITE_EXTENSION_ENTRY_POINT
#endif

using namespace std;
using namespace jsoncons;
namespace jmespath = jsoncons::jmespath;
namespace jsonpatch = jsoncons::jsonpatch;
namespace jsonschema = jsoncons::jsonschema; 

// Inspired by https://github.com/0x09/sqlite-statement-vtab/blob/master/statement_vtab.c
#ifdef SQLITE_CORE
#define sqlite_jsoncons_entry_point sqlite_jsoncons_init
#else
#define sqlite_jsoncons_entry_point sqlite3_extension_init
#endif

// This needs to be callable from C
extern "C" SQLITE_EXTENSION_ENTRY_POINT int sqlite_jsoncons_entry_point(
    sqlite3 *db,
    char **pzErrMsg,
    const sqlite3_api_routines *pApi);




static void flatten_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc == 1);
  nlohmann::json source;



  if (sqlite3_value_type(argv[0]) == SQLITE_NULL)
  {
    sqlite3_result_null(context);
    return;
  }

  try{
    source = nlohmann::json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
    auto r = source.flatten().dump();
    sqlite3_result_text(context, r.data(), (int)r.length(), SQLITE_TRANSIENT);
  }
  catch (std::exception e)
  {
    std::string message = e.what();
    sqlite3_result_error(context, message.data(), (int)message.length());
    return;
  }
}



static void unflatten_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc == 1);
  nlohmann::json source;



  if (sqlite3_value_type(argv[0]) == SQLITE_NULL)
  {
    sqlite3_result_null(context);
    return;
  }

  try{
    source = nlohmann::json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
    auto r = source.unflatten().dump();
    sqlite3_result_text(context, r.data(), (int)r.length(), SQLITE_TRANSIENT);
  }
  catch (std::exception e)
  {
    std::string message = e.what();
    sqlite3_result_error(context, message.data(), (int)message.length());
    return;
  }
}

static void patch_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc == 2);
  nlohmann::json source, target,patch;
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL){
    source = nlohmann::json(nullptr);
  } else {
     source = nlohmann::json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
  }
  if (sqlite3_value_type(argv[1]) == SQLITE_NULL){
    patch = nlohmann::json(nullptr);
  } else {
    patch = nlohmann::json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[1])));
  }

  target = source.patch(patch);
  if (target.is_null()) {
      sqlite3_result_null(context);
  } else {
      auto r = target.dump();
      sqlite3_result_text(context, r.data(), (int)r.length(), SQLITE_TRANSIENT);
  }
}

static void diff_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc == 2);

  // NULL can be special-cased
  // I don't know the SQLite types well enough nor  modern C++ to do this
  // more succinctly.
  nlohmann::json source, target;
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL){
    source = nlohmann::json(nullptr);
  } else {
     source = nlohmann::json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
  }
  if (sqlite3_value_type(argv[1]) == SQLITE_NULL){
    target = nlohmann::json(nullptr);
  } else {
    target = nlohmann::json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[1])));
  }

  nlohmann::json  patch = nlohmann::json::diff(source, target);
  auto r = patch.dump();
  sqlite3_result_text(context, r.data(), (int)r.length(), SQLITE_TRANSIENT);
  return;
}

// TODO: surface the from_diff, apply_patch from jsoncons::jsonpatch
// perhaps
//  https://json.nlohmann.me/api/basic_json/diff/
//  https://json.nlohmann.me/api/basic_json/patch/

static void jmespath_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc >= 2);

  if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
      sqlite3_value_type(argv[1]) == SQLITE_NULL)
  {
    sqlite3_result_null(context);
    return;
  }

  std::string j_text, expr_text;

  j_text = (reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
  expr_text = (reinterpret_cast<const char *>(sqlite3_value_text(argv[1])));

  json result;

  try
  {
    // TODO: parse and cache (sqlite3_{get,set}_auxdata) the expression
    result = jmespath::search(json::parse(j_text), expr_text);
  }
  catch (std::exception e)
  {
    std::string message = e.what();
    sqlite3_result_error(context, message.data(), (int)message.length());
    return;
  }


  if (result == json::null())
  {
    sqlite3_result_null(context);
  }
  else
  {
    // TODO: see how to return result without as_string
    std::string result_as_string = result.as_string();
    sqlite3_result_text(context, result_as_string.data(), (int)result_as_string.length(), SQLITE_TRANSIENT);
  }

  return;
}

static void from_diff_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc == 2);

  json source, target;
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL){
    source =  json::null();
  } else {
    source = json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
  }
  if (sqlite3_value_type(argv[1]) == SQLITE_NULL){
    target = json::null();
  } else {
    target = json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[1])));
  }

  json  patch = jsonpatch::from_diff(source, target);
  auto r = patch.as_string();
  sqlite3_result_text(context, r.data(), (int)r.length(), SQLITE_TRANSIENT);
  return;
}

static void apply_patch_func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv)
{
  assert(argc == 2);

  json target, patch;
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL){
    target =  json::null();
  } else {
    target = json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[0])));
  }
  if (sqlite3_value_type(argv[1]) == SQLITE_NULL){
    patch = json::null();
  } else {
    patch = json::parse(reinterpret_cast<const char *>(sqlite3_value_text(argv[1])));
  }

  try {
    jsonpatch::apply_patch(target,patch);
    auto r = target.as_string();
    sqlite3_result_text(context, r.data(), (int)r.length(), SQLITE_TRANSIENT);
  }
  catch (std::exception e)
  {
    std::string message = e.what();
    sqlite3_result_error(context, message.data(), (int)message.length());
    return;
  }
}

int sqlite_jsoncons_entry_point(
    sqlite3 *db,
    char **pzErrMsg,
    const sqlite3_api_routines *pApi)
{
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  (void)pzErrMsg; /* Unused parameter */

  rc = sqlite3_create_function(db, "jmespath_search", 2,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, jmespath_func, 0, 0);

  rc = sqlite3_create_function(db, "jmespath_search", 3,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, jmespath_func, 0, 0);

  rc = sqlite3_create_function(db, "json_diff", 2,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, diff_func, 0, 0);

  rc = sqlite3_create_function(db, "json_patch", 2,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, patch_func, 0, 0);

  rc = sqlite3_create_function(db, "json_from_diff", 2,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, from_diff_func, 0, 0);

  rc = sqlite3_create_function(db, "json_apply_patch", 2,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, apply_patch_func, 0, 0);

  rc = sqlite3_create_function(db, "json_flatten", 1,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, flatten_func, 0, 0);
  rc = sqlite3_create_function(db, "json_unflatten", 1,
                               SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                               0, unflatten_func, 0, 0);

  return rc;
}