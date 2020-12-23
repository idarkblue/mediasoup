SERVER=$1
STREAM=$2
curl -H "Content-Type:application/json" -X POST -d "{ \"stream\": \"${STREAM}\", \"method\":\"record.stop\", \"data\": { } }" $SERVER
