# Jewels - Jogo em C com Allegro 5
Este é um jogo desenvolvido em **C** utilizando a biblioteca **Allegro 5** para gráficos, áudio e fontes.  
O projeto utiliza um **Makefile** para automatizar a compilação.

## Dependências
Antes de compilar, é necessário instalar as bibliotecas de desenvolvimento do Allegro 5 e o `pkg-config`.

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install liballegro5-dev liballegro-image5-dev liballegro-ttf5-dev \
                 liballegro-font5-dev liballegro-audio5-dev liballegro-acodec5-dev \
                 liballegro-primitives5-dev pkg-config make gcc
```
### Archlinux / Manjaro
```
sudo pacman -S allegro make gcc pkgconf
```

## Compilação
No diretório raiz do projeto, execute:
```bash
make
```
Isso irá compilar todos os arquivos .c da pasta src/ e gerar o executável:
./jewels
