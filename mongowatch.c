#include <mongoc.h>

#define b64_ntop __b64_ntop

int b64_ntop(unsigned char const *src, size_t srclength, char *target, size_t targsize);

int main (int argc, char *argv[])
{
    if( argc != 4 ) {
        printf("Usage: mongowatcher MONGOURI dbname collection");
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

    bson_t empty = BSON_INITIALIZER;
    const char *resumeToken = "glo28TEAAAABRmRfaWQAZFo28TG9tDIr8SOf4gBaEAQu8VKGVklIebgnfVXEraDhBAAC";
    BSON_APPEND_UTF8(&empty, "resumeAfter", resumeToken);
    stream = mongoc_collection_watch (coll, &empty, NULL);

    /*
    bson_t *to_insert = BCON_NEW ("x", BCON_INT32 (1));
    bson_t opts = BSON_INITIALIZER;
    bool r;

    mongoc_write_concern_t *wc = mongoc_write_concern_new ();
    mongoc_write_concern_set_wmajority (wc, 10000);
    mongoc_write_concern_append (wc, &opts);
    r = mongoc_collection_insert_one (coll, to_insert, &opts, NULL, &err);
    if (!r) {
       fprintf (stderr, "Error: %s\n", err.message);
       return 1;
    }
    bson_destroy (to_insert);
    mongoc_write_concern_destroy (wc);
    bson_destroy (&opts);
    */
    char str[25];
    const bson_oid_t *oid = NULL;
    size_t b64_len;
    char *b64;
    const uint8_t *binary = NULL;
    uint32_t key_len = 0;

    while (true) {

        if (mongoc_change_stream_next (stream, &doc)) {
            if (bson_iter_init (&iter, doc) &&
                    bson_iter_find_descendant (&iter, "documentKey._id", &sub_iter)) {

                oid = bson_iter_oid (&sub_iter);
                bson_oid_to_string (oid, str);
                printf("DocID=%s\t\t", str);

            }

            if (bson_iter_init (&iter, doc) &&
                    bson_iter_find_descendant (&iter, "_id._data", &sub_iter)) {

                bson_iter_binary (&sub_iter, &btype, &key_len, (const uint8_t **) &binary);

                b64_len = (key_len / 3 + 1) * 4 + 1;
                b64 = bson_malloc0 (b64_len);

                b64_ntop (binary, b64_len, b64, b64_len);
                printf("ResumeToken= %s\n", b64);
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
