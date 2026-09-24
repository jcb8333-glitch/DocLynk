#!/bin/bash
k3d cluster start testnet
docker network connect k3d-testnet $(hostname) || true
kubectl get nodes

set -e
OPS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$OPS_DIR")"

docker build -t p2p-node:latest "$ROOT_DIR"
k3d image import p2p-node:latest -c testnet
kubectl apply -f "$OPS_DIR/testnet.yaml"
kubectl -n testnet rollout restart deploy/bootnode deploy/node
kubectl -n testnet rollout status deploy/node
kubectl -n testnet get pods -o wide