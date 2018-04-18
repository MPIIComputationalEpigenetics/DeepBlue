import xmlrpclib
import time
from pprint import pprint

DEEPBLUE_URL = "http://localhost:31415"
USER_KEY = 'anonymous_key'

def request_data(request_id):
    (status, info) = SERVER.info(request_id, USER_KEY)
    request_status = info[0]["state"]

    while request_status != "done" and request_status != "failed":
        print info
        time.sleep(1)
        (status, info) = SERVER.info(request_id, USER_KEY)
        request_status = info[0]["state"]

    (status, data) = SERVER.get_request_data(request_id, USER_KEY)
    if status != "okay":
        print data
        return None

    return data

if __name__ == "__main__":
    SERVER = xmlrpclib.Server(DEEPBLUE_URL, allow_none=True)
    print SERVER.echo(USER_KEY)

    _, QUERY_ID = SERVER.select_experiments("#c16368cfa854cd89b0e7a26599d3482c", None, None, None, USER_KEY)
    print QUERY_ID

    status, GET_REGIONS = SERVER.get_regions(QUERY_ID, "CHROMOSOME,START,END",USER_KEY)

    print status, GET_REGIONS

    ENRICHMENT_RESULT = request_data(GET_REGIONS)

    pprint(ENRICHMENT_RESULT)
