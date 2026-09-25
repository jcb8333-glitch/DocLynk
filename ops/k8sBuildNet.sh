#!/bin/bash
set -e
OPS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$OPS_DIR")"

docker build -t p2p-node:latest "$ROOT_DIR"
k3d image import p2p-node:latest -c testnet

kubectl apply -f "$OPS_DIR/testnet.yaml"
kubectl -n testnet rollout restart deploy/node-0
kubectl -n testnet rollout status deploy/node-0

kubectl -n testnet delete pod -l app=node --wait=true
for i in $(seq 63); do
  kubectl -n testnet create -f "$OPS_DIR/node-pod.yaml"
done

kubectl -n testnet get pods -o wide