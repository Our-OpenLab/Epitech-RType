#!/bin/bash

APP_NAME="rtype-core"
NAMESPACE="default"
IMAGE="mygame/server:latest"

# Étape 1 : Mettre à jour l'image (ou re-pull si nécessaire)
docker build -t $IMAGE .
docker push $IMAGE

# Étape 2 : Supprimer l'ancien déploiement (optionnel)
kubectl delete pod -n $NAMESPACE -l app=$APP_NAME --ignore-not-found=true

# Étape 3 : Appliquer les modifications
kubectl apply -f k8s/deployment.yaml
kubectl rollout restart deployment/$APP_NAME -n $NAMESPACE

# Étape 4 : Vérifier le statut
kubectl get pods -n $NAMESPACE
