import json


def next_move(event, context):
    data = json.loads(event['body'])

    response = {
        "statusCode": 200,
        "body": json.dumps(data)
    }

    return response

    # Use this code if you don't use the http event with the LAMBDA-PROXY
    # integration
    """
    return {
        "message": "Go Serverless v1.0! Your function executed successfully!",
        "event": event
    }
    """
