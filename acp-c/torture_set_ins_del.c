/*
 * acp-c : Arcus C Client Performance benchmark program
 * Copyright 2013-2014 NAVER Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <assert.h>

#include "libmemcached/memcached.h"
#include "common.h"
#include "keyset.h"
#include "valueset.h"
#include "client_profile.h"
#include "client.h"

static int
do_set_test(struct client *cli)
{
  memcached_coll_create_attrs_st attr;
  memcached_return rc;
  int ok, keylen, base;
  const char *key;
  uint8_t *val_ptr;
  int val_len;
  int skey;
  uint8_t val_buf[10];

  // Pick a key
  key = keyset_get_key(cli->ks, &base);
  keylen = strlen(key);
  
  // Create a set item
  if (0 != client_before_request(cli))
    return -1;

  memcached_coll_create_attrs_init(&attr, 10 /* flags */, 100 /* exptime */,
    4000 /* maxcount */);
  memcached_coll_create_attrs_set_overflowaction(&attr, OVERFLOWACTION_ERROR);
  rc = memcached_sop_create(cli->next_mc, key, keylen, &attr);
  ok = (rc == MEMCACHED_SUCCESS);
  if (!ok) {
    print_log("sop create failed. id=%d key=%s rc=%d(%s)", cli->id, key,
      rc, memcached_strerror(NULL, rc));
  }
  if (0 != client_after_request(cli, ok))
    return -1;

  // Insert 4000 elements and delete 4000
  
  for (skey = base; skey < base + 4000; skey++) {
    if (0 != client_before_request(cli))
      return -1;
    
    val_ptr = val_buf;
    val_len = sizeof(val_buf);
    memset(val_ptr, 0, val_len);

    int n = skey;
    int i = 0;
    while (n != 0 && i < val_len) {
      val_ptr[i] = (n % 10);
      n = n / 10;
      i++;
    }
    
    rc = memcached_sop_insert(cli->next_mc, key, keylen,
      (const char*)val_ptr, (size_t)val_len,
      NULL /* Do not create automatically */);
    ok = (rc == MEMCACHED_SUCCESS);
    if (!ok) {
      print_log("sop insert failed. id=%d key=%s val_len=%d rc=%d(%s)",
        cli->id, key, val_len, rc, memcached_strerror(NULL, rc));
    }
    if (0 != client_after_request(cli, ok))
      return -1;
  }
  
  for (skey = base; skey < base + 4000; skey++) {
    if (0 != client_before_request(cli))
      return -1;
    
    val_ptr = val_buf;
    val_len = sizeof(val_buf);
    memset(val_ptr, 0, val_len);

    int n = skey;
    int i = 0;
    while (n != 0 && i < val_len) {
      val_ptr[i] = (n % 10);
      n = n / 10;
      i++;
    }
    
    rc = memcached_sop_delete(cli->next_mc, key, keylen,
      (const char*)val_ptr, (size_t)val_len,
      true /* drop_if_empty */);
    ok = (rc == MEMCACHED_SUCCESS);
    if (!ok) {
      print_log("sop delete failed. id=%d key=%s val_len=%d rc=%d(%s)",
        cli->id, key, val_len, rc, memcached_strerror(NULL, rc));
    }
    if (0 != client_after_request(cli, ok))
      return -1;
  }
  
  return 0;
}

static int
do_test(struct client *cli)
{
  if (0 != do_set_test(cli))
    return -1; // Stop the test
  
  return 0; // Do another test
}

static struct client_profile default_profile = {
  .do_test = do_test,
};

struct client_profile *
torture_set_ins_del_init(void)
{
  return &default_profile;
}
