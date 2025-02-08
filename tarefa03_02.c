#include <stdio.h>                    // Biblioteca padrão de entrada e saída
#include "pico/stdlib.h"              // Biblioteca padrão para Raspberry Pi Pico
#include <ctype.h>                    // Biblioteca para manipulação de caracteres
#include <string.h>                   // Biblioteca para manipulação de strings
#include "hardware/pio.h"             // Biblioteca para uso do PIO (Programável I/O)
#include "hardware/timer.h"           // Biblioteca para manipulação de timers
#include <hardware/i2c.h>             // Biblioteca para comunicação I2C
#include <hardware/clocks.h>          // Biblioteca para manipulação dos clocks
#include "build/tarefa03_02.pio.h"    // Programa PIO específico para controlar a matriz de LEDs
#include <src/ssd1306.h>              // Biblioteca para controle do display OLED SSD1306
#include <src/font.h>                 // Fonte para o display OLED

#define NUM_LEDS 25                // Quantidade de LEDs na matriz (5x5)
#define PINO_MATRIZ 7              // Pino GPIO conectado à matriz de LEDs

#define LED_VERMELHO 13            // Pino GPIO do LED Vermelho
#define LED_VERDE 11               // Pino GPIO do LED Verde
#define LED_AZUL 12                // Pino GPIO do LED Azul

#define BOTAO_A 5                  // Pino GPIO do Botão A
#define BOTAO_B 6                  // Pino GPIO do Botão B

#define DEBOUNCE_TIME_MS 200       // Tempo de debounce para evitar leituras falsas dos botões (200ms)

#define I2C_SDA 14                 // Pino SDA do I2C
#define I2C_SCL 15                 // Pino SCL do I2C
#define ENDERECO_I2C 0x3c          // Endereço I2C do display SSD1306
#define PORTA_I2C i2c1             // Porta I2C utilizada (i2c1)

// Variáveis globais
static uint8_t matriz[NUM_LEDS][3];    // Matriz para armazenar o estado de cada LED (R, G, B)
static PIO pio_matriz;                 // Controlador PIO usado para a matriz de LEDs
static uint maquina;                   // Máquina de estado PIO usada para controlar a matriz
static volatile int numero_atual = 0;  // Número atual exibido na matriz (0 a 9)
static int estado_led_verde = 0;       // Estado do LED Verde (0 = apagado, 1 = aceso)
static int estado_led_azul = 0;        // Estado do LED Azul (0 = apagado, 1 = aceso)

// Mensagens exibidas no display OLED para o status dos LEDs
static char azul_mensagem[40]= "Led Azul Off"; 
static char verde_mensagem[40] = "Led Verde Off";

// Estrutura do display OLED
ssd1306_t ssd;

// Representação dos números na matriz de LEDs (cada número é uma matriz 5x5)
int representacao_numeros[10][NUM_LEDS] = {
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0},
    {0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0},
    {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}
};

// Protótipos das funções a serem usadas
void iniciar_botoes_leds();
void iniciar_i2c();
void iniciar_display(ssd1306_t *ssd);
void inicializar_matriz(uint pino);
void limpar_matriz();
void desenhar_numero(int numero, int cor);
void atualizar_matriz();
void atualizar_display();
static void interrupcao_botao(uint gpio, uint32_t eventos);

// Função principal 
int main() {
    stdio_init_all(); // Inicializa a comunicação USB

    iniciar_botoes_leds(); // Configura os botões e LEDs
    iniciar_i2c();         // Configura a comunicação I2C
    iniciar_display(&ssd); // Inicializa o display OLED

    inicializar_matriz(PINO_MATRIZ); // Inicializa a matriz de LEDs
    limpar_matriz();                 // Limpa a matriz (todos os LEDs apagados)

    // Configura as interrupções para os botões A e B
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &interrupcao_botao);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &interrupcao_botao);

    bool color = true;
    char caractere = ' ';
    ssd1306_fill(&ssd, !color);
    ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color);
    ssd1306_draw_char(&ssd, caractere, 58, 30);
    ssd1306_draw_string(&ssd, verde_mensagem, 10, 10);
    ssd1306_draw_string(&ssd, azul_mensagem, 10, 50);
    ssd1306_send_data(&ssd);

    while (true) {
        if(stdio_usb_connected()) {
            if (scanf("%c", &caractere) == 1) {
                printf("Caractere digitado: %c\n", caractere);

                // Desenha na matriz de led se for um número
                if (isdigit(caractere)) {
                    int numero = caractere - '0'; // Converte o caractere para o número correspondente
                    desenhar_numero(numero, 1);   // Desenha o número na matriz (1 para acender o LED)
                }
                else {
                    limpar_matriz();
                    atualizar_matriz();
                }
            }

            color = !color;

            // Atualiza o conteúdo do display com animações
            ssd1306_fill(&ssd, !color); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color); // Desenha um retângulo
            ssd1306_draw_char(&ssd, caractere, 58, 30);
            ssd1306_draw_string(&ssd, verde_mensagem, 10, 10);
            ssd1306_draw_string(&ssd, azul_mensagem, 10, 50);
            ssd1306_send_data(&ssd); // Atualiza o display
        }

        sleep_ms(100);
    }

}

// Função para configurar os botões e LEDs
void iniciar_botoes_leds() {
    // Configura os botões como entrada com pull-up
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    
    // Configura os LEDs como saída
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
}

void iniciar_i2c() {
    i2c_init(PORTA_I2C, 400000);               // Inicializa a porta I2C especificada, configurando a frequência de 400 kHz
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL);                     // Ativa o resistor de pull-up no pino SDA
    gpio_pull_up(I2C_SDA);                     // Ativa o resistor de pull-up no pino SCL
}

void iniciar_display(ssd1306_t *ssd) {
    ssd1306_init(ssd, WIDTH, HEIGHT, false, ENDERECO_I2C, PORTA_I2C); // Inicializa o display
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);                                           // Envia os dados iniciais para o display
}

// Função para inicializar a matriz de LEDs usando PIO
void inicializar_matriz(uint pino) {
    uint offset = pio_add_program(pio0, &matriz_led_program); // Adiciona o programa PIO
    pio_matriz = pio0; // Usa o controlador PIO0

    maquina = pio_claim_unused_sm(pio_matriz, false); // Obtém uma máquina de estado livre
    if (maquina < 0) {
        pio_matriz = pio1; // Se não houver máquinas livres no PIO0, tenta no PIO1
        maquina = pio_claim_unused_sm(pio_matriz, true);
    }

    // Inicializa o programa PIO na máquina de estado
    matriz_led_program_init(pio_matriz, maquina, offset, pino, 800000.f);
}

// Função para limpar a matriz (todos os LEDs apagados)
void limpar_matriz() {
    for (int i = 0; i < NUM_LEDS; i++) {
        matriz[i][0] = 0; // Vermelho
        matriz[i][1] = 0; // Verde
        matriz[i][2] = 0; // Azul
    }
}

// Função para desenhar um número na matriz de LEDs com a cor especificada
void desenhar_numero(int numero, int cor) {
    limpar_matriz(); // Limpa a matriz antes de desenhar
    for (int i = 0; i < NUM_LEDS; i++) {
        if (representacao_numeros[numero][i]) {
            if (cor == 0) { // Se a cor for 0, usa vermelho
                matriz[i][0] = 15; // Vermelho
            } else if (cor == 1) { // Se a cor for 1, usa azul
                matriz[i][2] = 15; // Azul
            }
        }
    }
    atualizar_matriz(); // Atualiza a matriz com o novo desenho
}

// Função para enviar os dados da matriz para a máquina de estado PIO
void atualizar_matriz() {
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio_matriz, maquina, matriz[i][1]); // Verde
        pio_sm_put_blocking(pio_matriz, maquina, matriz[i][0]); // Vermelho
        pio_sm_put_blocking(pio_matriz, maquina, matriz[i][2]); // Azul
    }
}

void atualizar_display() {
    // Atualiza as mensagens de status dos LEDs
    if (estado_led_verde) {
        strcpy(verde_mensagem, "Led Verde: On");
        printf("%s\n", verde_mensagem);
    } else {
        strcpy(verde_mensagem, "Led Verde: Off");
        printf("%s\n", verde_mensagem);
    }

    if (estado_led_azul) {
        strcpy(azul_mensagem, "Led Azul: On");
        printf("%s\n", azul_mensagem);
    } else {
        strcpy(azul_mensagem, "Led Azul: Off");
        printf("%s\n", azul_mensagem);
    }

    // Atualiza o display OLED
    ssd1306_fill(&ssd, 0); // Limpa o display
    ssd1306_draw_string(&ssd, verde_mensagem, 10, 10); // Exibe o estado do LED Verde
    ssd1306_draw_string(&ssd, azul_mensagem, 15, 40);  // Exibe o estado do LED Azul
    ssd1306_send_data(&ssd); // Envia os dados para atualizar o display
}

// Função de interrupção para os botões
static void interrupcao_botao(uint gpio, uint32_t eventos) {
    if (gpio == BOTAO_A) {
        // Verifica o estado do LED verde e alterna
        if (estado_led_verde == 0) {
            estado_led_verde = 1;
            gpio_put(LED_VERDE, 1);
        } else {
            estado_led_verde = 0;
            gpio_put(LED_VERDE, 0);
        }
    } 
    else if (gpio == BOTAO_B) {
        // Verifica o estado do LED azul e alterna
        if (estado_led_azul == 0) {
            estado_led_azul = 1;
            gpio_put(LED_AZUL, 1);
        } else {
            estado_led_azul = 0;
            gpio_put(LED_AZUL, 0);
        }
    }
    atualizar_display();
}


