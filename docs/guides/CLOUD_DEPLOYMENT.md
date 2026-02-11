# EVE OFFLINE - Cloud Deployment Guide

This guide covers deploying the EVE OFFLINE dedicated server to popular cloud platforms.

## Table of Contents
1. [Docker Deployment](#docker-deployment)
2. [AWS Deployment](#aws-deployment)
3. [Google Cloud Platform](#google-cloud-platform)
4. [Digital Ocean](#digital-ocean)
5. [Azure Container Instances](#azure-container-instances)
6. [Performance Recommendations](#performance-recommendations)

---

## Docker Deployment

The server includes a multi-stage Dockerfile for optimized deployment.

### Build the Docker Image

```bash
cd /path/to/EVEOFFLINE
docker build -t eve-offline-server:latest .
```

### Run Locally

```bash
docker run -d \
  --name eve-server \
  -p 27015:27015 \
  -v $(pwd)/data:/app/data:ro \
  -v eve-server-saves:/app/saves \
  eve-offline-server:latest
```

### Configuration

Mount a custom config file:

```bash
docker run -d \
  --name eve-server \
  -p 27015:27015 \
  -v $(pwd)/data:/app/data:ro \
  -v $(pwd)/custom_config.json:/app/config/server_config.json:ro \
  -v eve-server-saves:/app/saves \
  eve-offline-server:latest
```

---

## AWS Deployment

### Option 1: AWS ECS (Elastic Container Service)

1. **Push image to ECR:**

```bash
# Authenticate to ECR
aws ecr get-login-password --region us-east-1 | \
  docker login --username AWS --password-stdin <account-id>.dkr.ecr.us-east-1.amazonaws.com

# Create repository
aws ecr create-repository --repository-name eve-offline-server

# Tag and push
docker tag eve-offline-server:latest <account-id>.dkr.ecr.us-east-1.amazonaws.com/eve-offline-server:latest
docker push <account-id>.dkr.ecr.us-east-1.amazonaws.com/eve-offline-server:latest
```

2. **Create ECS Task Definition:**

```json
{
  "family": "eve-offline-server",
  "networkMode": "awsvpc",
  "requiresCompatibilities": ["FARGATE"],
  "cpu": "1024",
  "memory": "2048",
  "containerDefinitions": [
    {
      "name": "eve-server",
      "image": "<account-id>.dkr.ecr.us-east-1.amazonaws.com/eve-offline-server:latest",
      "portMappings": [
        {
          "containerPort": 27015,
          "protocol": "tcp"
        }
      ],
      "logConfiguration": {
        "logDriver": "awslogs",
        "options": {
          "awslogs-group": "/ecs/eve-offline-server",
          "awslogs-region": "us-east-1",
          "awslogs-stream-prefix": "ecs"
        }
      }
    }
  ]
}
```

3. **Create ECS Service:**

```bash
aws ecs create-service \
  --cluster eve-cluster \
  --service-name eve-server \
  --task-definition eve-offline-server \
  --desired-count 1 \
  --launch-type FARGATE \
  --network-configuration "awsvpcConfiguration={subnets=[subnet-xxx],securityGroups=[sg-xxx],assignPublicIp=ENABLED}"
```

### Option 2: AWS EC2

1. **Launch Ubuntu EC2 instance** (t3.medium or larger)
2. **Install Docker:**

```bash
sudo apt-get update
sudo apt-get install -y docker.io
sudo systemctl enable docker
sudo systemctl start docker
```

3. **Deploy server:**

```bash
docker run -d \
  --restart unless-stopped \
  --name eve-server \
  -p 27015:27015 \
  -v /opt/eve-data:/app/data:ro \
  -v /opt/eve-saves:/app/saves \
  eve-offline-server:latest
```

4. **Configure Security Group** to allow TCP port 27015

---

## Google Cloud Platform

### Using Cloud Run

1. **Build and push to GCR:**

```bash
gcloud builds submit --tag gcr.io/<project-id>/eve-offline-server
```

2. **Deploy to Cloud Run:**

```bash
gcloud run deploy eve-server \
  --image gcr.io/<project-id>/eve-offline-server \
  --platform managed \
  --region us-central1 \
  --port 27015 \
  --allow-unauthenticated \
  --memory 2Gi \
  --cpu 1
```

**Note**: Cloud Run is designed for HTTP services. For TCP game servers, use GCE or GKE instead.

### Using Compute Engine (GCE)

1. **Create VM instance:**

```bash
gcloud compute instances create eve-server \
  --zone=us-central1-a \
  --machine-type=e2-medium \
  --image-family=ubuntu-2004-lts \
  --image-project=ubuntu-os-cloud \
  --boot-disk-size=20GB
```

2. **SSH and install Docker:**

```bash
gcloud compute ssh eve-server
sudo apt-get update && sudo apt-get install -y docker.io
```

3. **Run server:**

```bash
sudo docker run -d \
  --restart unless-stopped \
  --name eve-server \
  -p 27015:27015 \
  eve-offline-server:latest
```

4. **Create firewall rule:**

```bash
gcloud compute firewall-rules create allow-eve-server \
  --allow tcp:27015 \
  --target-tags eve-server
```

---

## Digital Ocean

### Using Droplets

1. **Create Droplet** (2GB RAM minimum, Docker pre-installed image)

2. **SSH to Droplet:**

```bash
ssh root@<droplet-ip>
```

3. **Deploy server:**

```bash
docker run -d \
  --restart unless-stopped \
  --name eve-server \
  -p 27015:27015 \
  -v /opt/eve-data:/app/data:ro \
  -v /opt/eve-saves:/app/saves \
  eve-offline-server:latest
```

4. **Configure firewall** via Digital Ocean dashboard to allow TCP 27015

### Using App Platform (Containerized Apps)

1. **Push to Docker Hub or DOCR:**

```bash
docker tag eve-offline-server:latest <dockerhub-user>/eve-offline-server:latest
docker push <dockerhub-user>/eve-offline-server:latest
```

2. **Create App** via Digital Ocean dashboard
3. **Configure**:
   - Source: Docker Hub
   - Port: 27015
   - Size: Basic ($12/month or higher)

**Note**: App Platform is HTTP-focused. For game servers, Droplets are recommended.

---

## Azure Container Instances

1. **Create resource group:**

```bash
az group create --name eve-server-rg --location eastus
```

2. **Push to Azure Container Registry:**

```bash
az acr create --resource-group eve-server-rg --name eveofflineregistry --sku Basic
az acr login --name eveofflineregistry
docker tag eve-offline-server:latest eveofflineregistry.azurecr.io/eve-offline-server:latest
docker push eveofflineregistry.azurecr.io/eve-offline-server:latest
```

3. **Deploy container instance:**

```bash
az container create \
  --resource-group eve-server-rg \
  --name eve-server \
  --image eveofflineregistry.azurecr.io/eve-offline-server:latest \
  --cpu 1 \
  --memory 2 \
  --ports 27015 \
  --protocol TCP \
  --ip-address Public
```

4. **Get public IP:**

```bash
az container show --resource-group eve-server-rg --name eve-server --query ipAddress.ip
```

---

## Performance Recommendations

### Minimum Requirements
- **CPU**: 1 vCPU
- **RAM**: 2GB
- **Storage**: 10GB
- **Network**: 10 Mbps

### Recommended (10-20 players)
- **CPU**: 2 vCPU
- **RAM**: 4GB
- **Storage**: 20GB SSD
- **Network**: 100 Mbps

### High Performance (20+ players)
- **CPU**: 4 vCPU
- **RAM**: 8GB
- **Storage**: 50GB SSD
- **Network**: 1 Gbps

### Cost Estimates (Monthly)

| Provider | Instance Type | vCPU | RAM | Cost |
|----------|--------------|------|-----|------|
| AWS | t3.medium | 2 | 4GB | ~$30 |
| GCP | e2-medium | 2 | 4GB | ~$25 |
| Azure | B2s | 2 | 4GB | ~$35 |
| Digital Ocean | 2GB Droplet | 1 | 2GB | $12 |
| Digital Ocean | 4GB Droplet | 2 | 4GB | $24 |

### Monitoring

Monitor these metrics for optimal performance:

1. **Server Metrics** (built-in via ServerMetrics):
   - Tick rate (should stay at target 20 TPS)
   - Entity count
   - Player count
   - System uptime

2. **System Metrics**:
   - CPU usage (should stay below 80%)
   - RAM usage (should have 20% free)
   - Network bandwidth
   - Disk I/O

3. **Application Logs**:
   - Check `/app/logs/server.log` for errors
   - Monitor for connection issues
   - Watch for performance degradation

### Backup Strategy

1. **Save Files**: Back up `/app/saves` directory regularly
2. **Configuration**: Version control `config/server_config.json`
3. **Frequency**: Daily backups recommended
4. **Retention**: Keep 7-30 days of backups

```bash
# Example backup script
#!/bin/bash
BACKUP_DIR="/backups/eve-server"
DATE=$(date +%Y%m%d_%H%M%S)

docker exec eve-server tar czf /tmp/saves_$DATE.tar.gz /app/saves
docker cp eve-server:/tmp/saves_$DATE.tar.gz $BACKUP_DIR/
docker exec eve-server rm /tmp/saves_$DATE.tar.gz

# Keep only last 30 days
find $BACKUP_DIR -name "saves_*.tar.gz" -mtime +30 -delete
```

---

## Troubleshooting

### Server Won't Start

Check logs:
```bash
docker logs eve-server
```

Common issues:
- Port 27015 already in use
- Insufficient memory
- Missing or corrupted data files

### Connection Issues

1. **Verify port is open:**
```bash
telnet <server-ip> 27015
```

2. **Check firewall rules**
3. **Verify security group/firewall allows TCP 27015**

### Performance Issues

1. **Check CPU/RAM usage:**
```bash
docker stats eve-server
```

2. **Review server logs for warnings**
3. **Consider upgrading instance size**
4. **Reduce tick rate or max players in config**

---

## Security Best Practices

1. **Use firewall rules** to restrict port 27015 to known IPs when possible
2. **Keep Docker images updated** regularly
3. **Use SSL/TLS** for admin interfaces (future feature)
4. **Enable whitelist** for private servers
5. **Regular backups** of save data
6. **Monitor logs** for suspicious activity

---

## Additional Resources

- [EVE OFFLINE Documentation](../README.md)
- [Server Configuration Guide](../../cpp_server/docs/SERVER_CONFIG.md)
- [Troubleshooting Guide](TROUBLESHOOTING.md)
- [Docker Documentation](https://docs.docker.com/)

---

*Last Updated: February 11, 2026*
