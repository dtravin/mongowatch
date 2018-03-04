#include <mongoc.h>

#define b64_ntop __b64_ntop

int b64_ntop(unsigned char const *src, size_t srclength, char *target, size_t targsize);

int main (int argc, char *argv[])
{
    if( argc < 4 ) {
        printf("Usage: mongowatcher MONGOURI dbname collection resumetoken");
        exit(-1);
    }

    const bson_t *doc;
    const bson_t *err_doc;
    mongoc_client_t *client;
    mongoc_collection_t *coll;
    mongoc_change_stream_t *stream;
    bson_error_t err;

    bson_iter_t iter;
    bson_iter_t sub_iter;

    bson_subtype_t btype;
    uint32_t buflen = 0;
    const char *tmpstr;

    mongoc_init ();

    client = mongoc_client_new(argv[1]);

    if (!client) {
        fprintf (stderr, "Cannot connect to replica set\n");
        return 1;
    }

    coll = mongoc_client_get_collection (client, argv[2], argv[3]);

    // TODO pipeline can filter out non-insert operations
    bson_t *pipeline = bson_new ();
    bson_t opts = BSON_INITIALIZER;

    if (argc == 5) {
      bson_error_t error;
      bson_t      *_data;

      char buffer[200];
      // some other options are
      //  { 'fullDocument': 'updateLookup', 'resumeAfter': {'_id': 0 }, 'maxAwaitTimeMS': 5000, 'batchSize': 5, 'collation': { 'locale': 'en' }}"
      // { "_data" : { "$binary" : { "base64": "glqZp2kAAAABRmRfaWQAZFqZp2kfgYxhx5KoeQBaEASGFlncanZNuKgd3gVcBudQBA==", "subType" : "00" } } }

      sprintf(buffer, "{\"_data\": {\"$binary\": {\"base64\": \"%s\", \"subType\": \"00\"}}}", argv[4]);
      _data = bson_new_from_json ((const uint8_t *)buffer, -1, &error);

      if (!_data) {
        fprintf (stderr, "Cannot use resume token %s with %s\n", error.message, buffer);
      }

      BSON_APPEND_DOCUMENT(&opts, "resumeAfter", _data);
    }

    stream = mongoc_collection_watch (coll, pipeline, &opts);

    char str[25];
    const bson_oid_t *oid = NULL;
    size_t b64_len;
    char *b64;
    const uint8_t *binary = NULL;
    uint32_t key_len = 0;

    while (true) {

        if (mongoc_change_stream_next (stream, &doc)) {

          /*
            char *as_json = bson_as_relaxed_extended_json (doc, NULL);
            fprintf (stderr, "Got document: %s\n", as_json);
            bson_free (as_json);
          */
            if (bson_iter_init (&iter, doc) &&
                bson_iter_find_descendant (&iter, "documentKey._id", &sub_iter)) {

                printf("%s\t", bson_iter_utf8 (&sub_iter, NULL));

            }

            if (bson_iter_init (&iter, doc) &&
                    bson_iter_find_descendant (&iter, "_id._data", &sub_iter)) {

                bson_iter_binary (&sub_iter, &btype, &key_len, (const uint8_t **) &binary);

                b64_len = (key_len / 3 + 1) * 4 + 1;
                b64 = bson_malloc0 (b64_len);

                b64_ntop (binary, b64_len, b64, b64_len);
                printf("%s\n", b64);
                fflush(stdout);
            }
        }
        if (mongoc_change_stream_error_document (stream, &err, &err_doc)) {
            if (!bson_empty (err_doc)) {
                fprintf (stderr,
                         "Server Error: %s\n",
                         bson_as_relaxed_extended_json (err_doc, NULL));
            } else {
                fprintf (stderr, "Client Error: %s\n", err.message);
            }
            return 1;
        }
    }

    bson_free (b64);
    mongoc_change_stream_destroy (stream);
    mongoc_collection_destroy (coll);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
}
