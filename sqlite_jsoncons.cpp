
#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <iostream>

#ifdef WIN32
#define SQLITE_EXTENSION_ENTRY_POINT __declspec(dllexport)
#else
#define SQLITE_EXTENSION_ENTRY_POINT
#endif

using namespace std;
using namespace jsoncons;
namespace jmespath = jsoncons::jmespath;

// This needs to be callable from C
extern "C" SQLITE_EXTENSION_ENTRY_POINT int sqlite3_extension_init(
    sqlite3 *db,
    char **pzErrMsg,
    const sqlite3_api_routines *pApi);

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

int sqlite3_extension_init(
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
  return rc;
}