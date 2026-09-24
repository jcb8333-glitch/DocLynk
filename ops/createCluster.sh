#!/bin/bash
set -e
k3d cluster create testnet --k3s-arg "--tls-san=k3d-testnet-serverlb@server:*"
docker network connect k3d-testnet $(hostname) || true
kubectl config set-cluster k3d-testnet --server=https://k3d-testnet-serverlb:6443
kubectl get nodes