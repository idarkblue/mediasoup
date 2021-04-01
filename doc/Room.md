# protocol

## 1. 客户端发起的请求

### 1.1 joinRoom

```json
{
    "version": 0,
    "cseq": 0,
    "method": "joinRoom",
    "userId": "xxx",
    "roomId": "xxx",
    "data": {
    }
}
```

- 返回

```json
{
    "version": 0,
    "cseq": 0,
    "code": 0,
    "result": 0,
    "data": {
        "streamList": [
            {
                "userId": "xxx",
                "streamId": "xxx"
            }
        ]
    }
}
```

### 1.2 publishStream

```json
{
    "version": 0,
    "cseq": 0,
    "method": "publishStream",
    "userId": "xxx",
    "roomId": "xxx",
    "data": {
        "streamId": "xxx"
    }
}
```

- 返回

```json
{
    "version": 0,
    "cseq": 0,
    "code": 0,
    "result": 0,
    "data": {
    }
}
```

## 2. 服务器主动发出的通知事件

### 2.1 streamStat

```json
{
    "version": 0,
    "cseq": 0,
    "method": "streamStat",
    "userId": "xxx",
    "roomId": "xxx",
    "data": {
        "streamId": "xxx",
        "stat": "start/stop"
    }
}
```
