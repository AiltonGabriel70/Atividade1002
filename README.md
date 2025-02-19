Instruções do Programa: Sistema de Controle com Joystick e Display OLED

Este programa controla um sistema que utiliza um joystick para movimentar um quadrado na tela de um display OLED e ajusta a intensidade de LEDs RGB com base na posição do joystick. Além disso, há botões para interagir com o sistema, como ligar/desligar o LED verde e habilitar/desabilitar o controle PWM dos LEDs.

Funcionalidades Principais
Controle do Joystick:

O joystick tem dois eixos (X e Y) que são lidos via ADC (Conversor Analógico-Digital).

A posição do joystick é mapeada para mover um quadrado na tela do display OLED.

A intensidade dos LEDs vermelho e azul é ajustada com base na posição do joystick.

Display OLED:

Um quadrado é desenhado na tela do display OLED, movendo-se conforme a posição do joystick.

O display tem uma borda dupla e o quadrado é centralizado inicialmente.

Controle dos LEDs RGB:

O LED vermelho é controlado pelo eixo X do joystick.

O LED azul é controlado pelo eixo Y do joystick.

O LED verde pode ser ligado/desligado pressionando o botão do joystick.

O controle PWM dos LEDs pode ser habilitado/desabilitado pressionando o botão A.

Botões:

Botão do Joystick: Liga/desliga o LED verde.

Botão A: Habilita/desabilita o controle PWM dos LEDs vermelho e azul.

Configurações de Hardware
Joystick:

Eixo X: Conectado ao pino GPIO 26 (ADC0).

Eixo Y: Conectado ao pino GPIO 27 (ADC1).

Botão: Conectado ao pino GPIO 22.

Botão A:

Conectado ao pino GPIO 5.

LEDs RGB:

LED Vermelho: Conectado ao pino GPIO 12.

LED Verde: Conectado ao pino GPIO 11.

LED Azul: Conectado ao pino GPIO 13.

Display OLED:

Conectado via I2C (SDA no pino GPIO 14, SCL no pino GPIO 15).

Endereço I2C: 0x3C.

Resolução: 128x64 pixels.

Funcionamento do Programa
Inicialização:

O programa configura os pinos do joystick, botões, LEDs e display OLED.

O PWM dos LEDs é configurado para controlar a intensidade de cada cor.

O display OLED é inicializado e exibe uma borda dupla com um quadrado centralizado.

Leitura do Joystick:

O programa lê os valores dos eixos X e Y do joystick via ADC.

Esses valores são mapeados para a posição do quadrado na tela e para a intensidade dos LEDs.

Atualização do Display:

O quadrado é movido na tela com base na posição do joystick.

O display é atualizado continuamente para refletir a nova posição do quadrado.

Controle dos LEDs:

A intensidade dos LEDs vermelho e azul é ajustada conforme a posição do joystick.

O LED verde é ligado/desligado ao pressionar o botão do joystick.

O controle PWM dos LEDs vermelho e azul é habilitado/desabilitado ao pressionar o botão A.

Debounce dos Botões:

O programa implementa um sistema de debounce para evitar leituras múltiplas ao pressionar os botões.

Instruções de Uso
Movimentação do Quadrado:

Mova o joystick para os lados (eixo X) e para cima/baixo (eixo Y) para mover o quadrado na tela.

Controle dos LEDs:

O LED vermelho brilha mais forte ao mover o joystick para os lados.

O LED azul brilha mais forte ao mover o joystick para cima/baixo.

Pressione o botão do joystick para ligar/desligar o LED verde.

Pressione o botão A para habilitar/desabilitar o controle PWM dos LEDs vermelho e azul.

Reset do Sistema:

Reinicie o programa para centralizar o quadrado e redefinir as configurações dos LEDs.

Explicação das Funções Principais
gpio_irq_handler:

Função de interrupção para tratar os eventos dos botões (joystick e botão A).

Implementa debounce para evitar leituras múltiplas.

setup_pwm:

Configura os pinos dos LEDs para funcionar com PWM.

update_leds:

Atualiza a intensidade dos LEDs com base na posição do joystick.

update_display:

Desenha o quadrado na tela do display OLED com base na posição do joystick.

handle_buttons:

Verifica se os botões foram pressionados e executa as ações correspondentes.

map_value:

Função auxiliar para mapear valores de uma faixa para outra (usada para mapear a posição do joystick para a tela e LEDs).

Considerações Finais

O código está otimizado para funcionar com o Raspberry Pi Pico.

Certifique-se de que o hardware esteja conectado corretamente antes de executar o programa.

O sistema é projetado para ser interativo e responsivo, com atualizações rápidas do display e dos LEDs.
