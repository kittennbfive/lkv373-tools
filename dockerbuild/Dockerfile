FROM archlinux
LABEL maintainer="github@danman.eu"

ENV GCC=nds32le-elf-gcc-9.3.0-1-x86_64.pkg.tar.xz
ENV NEWLIB=nds32le-elf-newlib-3.3.0-1-any.pkg.tar.xz

RUN curl https://github.com/kittennbfive/lkv373-tools/releases/download/0.01/$GCC -L -O
RUN curl https://github.com/kittennbfive/lkv373-tools/releases/download/0.01/$NEWLIB -L -O

RUN pacman -Suy --noconfirm
RUN pacman -U --needed --noconfirm $GCC $NEWLIB

