SQLite wrapper for JSON-related functions in jsoncons and nlohmann::json

Function include
  * jmespath_search (from jsoncons)
  * json_diff (from jsoncons)
  * json_patch (from jsoncons)
  * json_from_diff and json_apply_patch (from nlohmann::json)
  * json_flatten and json_unflatten (from nlohmann::json)


sqlite_jsoncons is distributed under the Boost Software License (same license used for jsoncons)


examples
========

From https://github.com/danielaparker/jsoncons/blob/master/doc/ref/jmespath/jmespath.md
```sql
select jmespath_search('{"people":[{"age":20,"other":"foo","name":"Bob"},{"age":25,"other":"bar","name":"Fred"},{"age":30,"other":"baz","name":"George"}]}', 'people[?age > `20`].[name, age]');
```
```json
[["Fred",25],["George",30]]
```