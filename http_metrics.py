import requests
import json
import time

USER_ID = "2336262"
API_KEY = "glc_eyJvIjoiMTM4MjAxMyIsIm4iOiJzdGFjay0xMjA2MDA5LWludGVncmF0aW9uLWVzcDMyX3BvbGljeSIsImsiOiJwMWg5MWlhUTIwRzcxM1BTNzlyRnF6aGQiLCJtIjp7InIiOiJwcm9kLXNhLWVhc3QtMSJ9fQ=="
timestamp = int(time.time())

body = [
    {
        "name": "test.metric",
        "interval": 1,
        "value": 15.3,
        "time": timestamp,
        "tags": ["tag=Kp", "source=grafana_cloud_docs"]
    },
    {
        "name": "test.metric",
        "interval": 1,
        "value": 12.7,
        "time": timestamp,
        "tags": ["tag=Kd", "source=grafana_cloud_docs"]
    },
    {
        "name": "test.metric",
        "interval": 1,
        "value": 11.5,
        "time": timestamp,
        "tags": ["tag=Ki", "source=grafana_cloud_docs"]
    },
    {
        "name": "test.metric",
        "interval": 1,
        "value": 9.4,
        "time": timestamp,
        "tags": ["tag=erro", "source=grafana_cloud_docs"]
    }
]

# Envia os dados
response = requests.post(
    'https://graphite-prod-40-prod-sa-east-1.grafana.net/graphite/metrics',
    headers={
        'Content-Type': 'application/json',
        'Authorization': f'Bearer {USER_ID}:{API_KEY}'
    },
    json=body
)

# Debug da resposta
print("Status Code:", response.status_code)
print("Headers:", response.headers)
print("Resposta Bruta:", response.text)

try:
    data = response.json()
    print("Resposta JSON:", data)
except json.JSONDecodeError as e:
    print(f"Erro ao decodificar JSON: {e}")
    print("Conteúdo bruto:", response.text)

