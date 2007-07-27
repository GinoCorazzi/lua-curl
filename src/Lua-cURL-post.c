/*
 * call-seq:
 *   easy.http_post("url=encoded%20form%20data;and=so%20on") => true
 *   easy.http_post("url=encoded%20form%20data", "and=so%20on", ...) => true
 *   easy.http_post("url=encoded%20form%20data", Curl::PostField, "and=so%20on", ...) => true
 *   easy.http_post(Curl::PostField, Curl::PostField ..., Curl::PostField) => true
 * 
 * POST the specified formdata to the currently configured URL using 
 * the current options set for this Curl::Easy instance. This method 
 * always returns true, or raises an exception (defined under 
 * Curl::Err) on error.
 * 
 * The Content-type of the POST is determined by the current setting
 * of multipart_form_post? , according to the following rules:
 * * When false (the default): the form will be POSTed with a 
 *   content-type of 'application/x-www-form-urlencoded', and any of the
 *   four calling forms may be used. 
 * * When true: the form will be POSTed with a content-type of 
 *   'multipart/formdata'. Only the last calling form may be used, 
 *   i.e. only PostField instances may be POSTed. In this mode,
 *   individual fields' content-types are recognised, and file upload
 *   fields are supported.
 * 
 */


/* only url-encoded at the moment */

#include "Lua-cURL.h"
#include "Lua-utility.h"

int l_easy_post(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  int index_next, index_table, index_key;
  const char *value, *key;

  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;

  /* param verification */
  luaL_checktable(L, 1);

  lua_getglobal(L, "pairs");
  lua_pushvalue(L, 1);
  lua_call(L, 1, 3);

  /* got next, t, k on stack */
  index_key = lua_gettop(L);
  index_table = index_key - 1;
  index_next = index_table - 1;

  while (1) {
    lua_pushvalue(L, index_next);
    lua_pushvalue(L, index_table);
    lua_pushvalue(L, index_key);

    lua_call(L, 2, 2);  
    if (lua_isnil(L, -2)) 
      break;

    /* duplicate key before converting to string (we need the original type for the iter function) */
    lua_pushvalue(L, -2);
    key = lua_tostring(L, -1); 
    lua_pop(L, 1);

    /* got {name= {file=blah,
                  {type="text/html"}
    */
    if (lua_istable(L, -1)) {
      const char* type = NULL;

      /* check for type option */
      lua_getfield(L, -1, "type");
      type = lua_isnil(L, -1)? NULL : lua_tostring(L, -1);
      lua_pop(L, 1);
      
      /* fileupload */
      lua_getfield(L, -1, "file");
      if (!lua_isnil(L, -1)) {
	const char *filename = lua_tostring(L, -1);
	
	/* Add file/contenttype section */
	if (((type == NULL)? 
	     curl_formadd(&post, &last, CURLFORM_COPYNAME, key,
			  CURLFORM_FILE, filename, CURLFORM_END):
	     curl_formadd(&post, &last, CURLFORM_COPYNAME, key,
			  CURLFORM_FILE, filename,
			  CURLFORM_CONTENTTYPE, type, CURLFORM_END)) != CURLE_OK)
	  luaL_error(L, "cannot set upload filename: %s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
      }
      else
	printf("Unnown\n");
      lua_pop(L, 1);
    }
    /* go name=value */
    else {
      value = luaL_checkstring(L, -1);
      
      /* add name/content section */
      curl_formadd(&post, &last, CURLFORM_COPYNAME, key, CURLFORM_COPYCONTENTS, value, CURLFORM_END);
    }
    
    /* remove value */
    lua_pop(L, 1);			
    /* move key */
    lua_replace(L, index_key);
  }
  
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
  return 0;
}
 /*  long value = luaL_checklong(L,1); */
/*     if ((rbce->postdata_buffer = rb_funcall(args_ary, idJoin, 1, rbstrAmp)) == Qnil) { */
/*       rb_raise(eCurlErrError, "Failed to join arguments"); */
/*       return Qnil; */
/*     } else {  */
/*       data = StringValuePtr(rbce->postdata_buffer);    */
/*       len = RSTRING_LEN(rbce->postdata_buffer); */
      
/*       curl_easy_setopt(curl, CURLOPT_POST, 1); */
/*       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); */
/*       curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len); */

/*       return handle_perform(rbce); */
/*     } */
/*   } */
/* } */
