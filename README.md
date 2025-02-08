# Projeto Tarefa Monitor Serial

Este repositório contém o código-fonte e as instruções para a implementação de um projeto com a plataforma BitDogLab, utilizando o microcontrolador RP2040. O objetivo principal é controlar LEDs RGB, botões com interrupções e debounce, além de exibir caracteres em um display SSD1306 via I2C. A comunicação serial é feita via UART, e a manipulação da matriz de LEDs WS2812 também é abordada.

## Componentes Utilizados

- **Placa BitDogLab** (com microcontrolador RP2040)
- **Matriz 5x5 de LEDs WS2812** conectada à GPIO 7
- **LED RGB** conectado às GPIOs 11, 12 e 13
- **Botões** A e B conectados às GPIOs 5 e 6, respectivamente
- **Display SSD1306** conectado via I2C (GPIO 14 e GPIO 15)
- **Comunicação Serial** via UART

## Como Baixar e Rodar o Projeto

Siga os passos abaixo para clonar e rodar o projeto localmente:

### Passos

1. **Clone o repositório:**

    ```bash
    git clone https://github.com/Difarias/tarefa03_02_UCC6
    ```

2. **Entre na pasta do repositório:**

    ```bash
    cd tarefa03_02_UCC6
    ```

3. **Crie a pasta de build e entre nela:**

    ```bash
    mkdir build
    cd build
    ```

4. **Gere os arquivos de build com CMake:**

    ```bash
    cmake ..
    ```

5. **Compile o projeto:**

    ```bash
    make
    ```

6. **Execute na placa**
    - Copie o arquivo .uf2 gerado na sua placa conectada ao PC.
    

## Demonstração

Confira a demonstração do funcionamento do projeto no vídeo abaixo:

[![Demonstração do Projeto](https://img.youtube.com/vi/4xROWoLU9Rk/0.jpg)](https://youtu.be/4xROWoLU9Rk)

## Funcionalidades Implementadas

- Controle de LEDs RGB através de botões com interrupções.
- Manipulação de matriz de LEDs WS2812 para exibição de padrões.
- Exibição de texto no display SSD1306 via I2C.
- Comunicação serial para controle e monitoramento dos componentes.
- Uso de interrupções e debounce para controle preciso dos botões.

## Desenvolvedor

Este projeto foi desenvolvido por **Diego Farias de Freitas**.
