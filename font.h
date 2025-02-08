#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Definição das fontes básicas para caracteres ASCII 0x20 a 0x5A (espaço até Z)
// Cada caractere é representado por uma matriz 5x7 pixels
static const uint8_t font[59][5] = {
    // Espaço (0x20)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // Caracteres !"#$%&'()*+,-./
    {0x00, 0x00, 0x5F, 0x00, 0x00},  // !
    {0x00, 0x07, 0x00, 0x07, 0x00},  // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    // ... (complete a tabela para os demais caracteres especiais)
    // Letras A-Z
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    // ... (adicione o restante das letras maiúsculas)
};

// Função para exibir um caractere no display SSD1306
void ssd1306_show_char(char c);

#endif // FONT_H
