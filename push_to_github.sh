#!/bin/bash

# Instrucciones para subir el código a GitHub
# Reemplaza TU_USUARIO con tu nombre de usuario de GitHub

echo "========================================"
echo "Subir ESP32 MusicBox a GitHub"
echo "========================================"
echo ""
echo "1. Ve a: https://github.com/new"
echo "2. Crea un repositorio llamado: esp32-musicbox"
echo "3. No inicialices con README/License/gitignore"
echo "4. Copia tu usuario de GitHub y pégalo aquí:"
read -p "Usuario de GitHub: " GITHUB_USER

echo ""
echo "Configurando remote..."
git remote add origin "https://github.com/${GITHUB_USER}/esp32-musicbox.git"

echo "Verificando rama..."
git branch -M main

echo ""
echo "Subiendo código a GitHub..."
git push -u origin main

echo ""
echo "========================================"
echo "¡Listo! Tu código está en:"
echo "https://github.com/${GITHUB_USER}/esp32-musicbox"
echo "========================================"
