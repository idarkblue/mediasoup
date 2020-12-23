SERVER=$1
STREAM=$2
FILE=$3
curl -H "Content-Type:application/json" -X POST -d "{ \"stream\": \"${STREAM}\", \"method\":\"record.start\", \"data\": { \"fileName\": \"${FILE}\" } }" $SERVER
