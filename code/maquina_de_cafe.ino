#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Definição do display LCD (endereço 0x27, 16 colunas e 2 linhas)
LiquidCrystal_I2C lcd(0x20, 16, 2);

// Definição dos pinos para os sensores
const int sensorTemperatura = A0; // Sensor TMP36
const int sensorBorra = A1;
const int sensorGraos = A3;  // Potenciômetro para medir os grãos
const int sensorAgua = A2;   // Potenciômetro para medir o nível de água

// Definição dos pinos para os motores e LED
const int motorMoagem = 13;
const int motorAgua = 10;
const int ledCafe = 11;
const int botaoReabastecerAgua = 12;

// Definição das variáveis do teclado 4x4
const byte FILAS = 4; // quatro filas
const byte COLUNAS = 4; // quatro colunas

char hexaKeys[FILAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinosFilas[FILAS] = {9, 8, 7, 6}; // Conectar às filas do teclado
byte pinosColunas[COLUNAS] = {5, 4, 3, 2}; // Conectar às colunas do teclado

Keypad teclado = Keypad(makeKeymap(hexaKeys), pinosFilas, pinosColunas, FILAS, COLUNAS);

// Variáveis de estado
float tempIdeal = 25.0; // Temperatura mínima para o café
int contadorCafes = 0;  // Contador de cafés feitos
bool maquinaParada = false; // Flag para pausar após 3 cafés
int nivelAgua = 30; // Nível inicial de água em ml
int valorAnteriorBotao = 0;
bool estadoBotao = LOW;

void setup() {
  // Inicializar o LCD
  lcd.init();
  // Ligar a retroiluminação do LCD
  lcd.backlight();
  
  pinMode(motorMoagem, OUTPUT);
  pinMode(motorAgua, OUTPUT);
  pinMode(ledCafe, OUTPUT);
  pinMode(botaoReabastecerAgua, INPUT);


  // Sensores como entradas
  pinMode(sensorTemperatura, INPUT);
  pinMode(sensorBorra, INPUT_PULLUP);
  pinMode(sensorGraos, INPUT);
  pinMode(sensorAgua, INPUT);

  // Exibir mensagem de boas-vindas
  lcd.print("Ola, Bem-vindo!");
  delay(3000); // Exibir por 3 segundos

  lcd.clear();
  lcd.print("Verificando...");
  delay(2000);
}

bool verificarCancelamento() {
  if (teclado.getKey() == '#') {
    lcd.clear();
    lcd.print("Operacao Cancelada");
    delay(2000);
    return true;
  }
  return false;
}

void loop() {
  // Reabastecer água se o botão for pressionado durante a seleção de café
  int valorBotao = digitalRead(botaoReabastecerAgua);
  if (valorBotao == HIGH && valorAnteriorBotao == LOW && nivelAgua <= 100) {
    nivelAgua += 50; // Adiciona 50ml ao nível de água
    if (nivelAgua > 100) nivelAgua = 100; // Garante que o nível máximo seja 100ml
    lcd.clear();
    lcd.print("Reabastecendo");
    lcd.setCursor(0,1);
    lcd.print("Agua: ");
    lcd.print(nivelAgua);
    lcd.print(" ml");
    delay(2000);
  }
  valorAnteriorBotao = valorBotao;

  // Verificação dos sensores antes de prosseguir
  if (!verificarSensores()) {
    return;  // Se algum sensor não estiver OK, a execução é interrompida.
  }

  // Pedir para selecionar o café
  lcd.clear();
  lcd.print("Selecionar Cafe");
  lcd.setCursor(0,1);
  lcd.print("ou Agua");
  char tecla = 0;

  // Esperar o botão ser pressionado
  while (true) {
    tecla = teclado.getKey();
    if (tecla) {
      break;
    }
    valorBotao = digitalRead(botaoReabastecerAgua);
    if (valorBotao == HIGH && valorAnteriorBotao == LOW && nivelAgua <= 100) {
      nivelAgua += 50; // Adiciona 50ml ao nível de água
      if (nivelAgua > 100) nivelAgua = 100; // Garante que o nível máximo seja 100ml
      lcd.clear();
      lcd.print("Reabastecendo");
      lcd.setCursor(0,1);
      lcd.print("Agua: ");
      lcd.print(nivelAgua);
      lcd.print(" ml");
      delay(2000);
      lcd.clear();
      lcd.print("Selecionar Cafe");
      lcd.setCursor(0,1);
      lcd.print("ou Agua");
    }
    valorAnteriorBotao = valorBotao;
  }

  int valorNecessario = 0;
  int consumo = 0;
  int tempoAgua = 0;

  // Definir consumo e tempo de água de acordo com o café selecionado
  switch(tecla) {
    case '1':
      valorNecessario = 50;
      consumo = 5;
      tempoAgua = 1000;
      lcd.clear();
      lcd.print("Expresso");
      lcd.setCursor(0,1);
      lcd.print("Selecionado");
      break;
    case '2':
      valorNecessario = 75;
      consumo = 10;
      tempoAgua = 2000;
      lcd.clear();
      lcd.print("Medio");
      lcd.setCursor(0,1);
      lcd.print("Selecionado");
      break;
    case '3':
     valorNecessario = 100;
      consumo = 15;
      tempoAgua = 3000;
      lcd.clear();
      lcd.print("Cheio");
      lcd.setCursor(0,1);
      lcd.print("Selecionado");
      break;
    case '4':  // Opção de "Água"
      valorNecessario = 0;
      consumo = 0;
      lcd.clear();
      lcd.print("Agua Quente");
      lcd.setCursor(0,1);
      lcd.print("Selecionada");
      // Mantenha o LED da água ligado indefinidamente até o usuário pressionar '1'
      lcd.clear();
      lcd.print("Saindo Agua...");
      digitalWrite(motorAgua, HIGH); // Liga a bomba de água (ou LED)
     
      // Loop infinito até o usuário pressionar '1'
      while (true) {
        char teclaParar = teclado.getKey();
        if (teclaParar == '1') {
          // Interrompe o ciclo de água ao pressionar '1'
          digitalWrite		(motorAgua, LOW);  // Desliga a bomba de água (ou LED
          lcd.clear();
          lcd.print("Agua Parada.");
          delay(2000); // Atraso para exibir mensagem de parada
          lcd.clear();
          lcd.print("Obrigado!");
          delay(2000); 
          break;  // Sai do loop e continua com o código
        }
      }
  return;  // Finaliza o processo de água quente e retorna ao início

    default:
      lcd.clear();
      lcd.print("Selec. Invalida");
      delay(2000);
      return; // Se uma tecla inválida for pressionada, retorna ao início
  }
  delay(1000); // Atraso para leitura

  // Inserir "moedas" para atingir o valor necessário
  int dinheiroInserido = 0;

  // Se for "Água", pula a parte de inserção de moedas e moagem
  if (tecla == '4') {
    // Processo imediato para "Água"
    lcd.clear();
    lcd.print("Processando Agua...");
    digitalWrite(motorAgua, HIGH); // Ativa a bomba de água
    delay(3000);  // Tempo padrão para água
    digitalWrite(motorAgua, LOW);  // Desativa a bomba de água
    lcd.clear();
    lcd.print("Agua Quente");
    lcd.setCursor(0,1);
    lcd.print("Pronta!");
    delay(3000);
    return;  // Finaliza o processo e retorna ao início
  }

  lcd.clear();
  lcd.print("Insira Dinheiro");

  while (true) { // Loop para inserção de moedas
    if (verificarCancelamento()) return;  // Cancela e volta ao início
    
    char moeda = teclado.getKey();
    if (moeda) {
      if (moeda == '*') { // Finalizar inserção ao pressionar '*'
        break;
      }

      switch (moeda) {
        case 'A': // Moeda de valor 0
          break;
        case 'B': // Moeda de valor 50
          dinheiroInserido += 50;
          break;
        case 'C': // Moeda de valor 75
          dinheiroInserido += 75;
          break;
        case 'D': // Moeda de valor 100
          dinheiroInserido += 100;
          break;
        default: // Caso insira uma tecla inválida
          continue;
      }

      // Atualizar o valor total inserido no display
      lcd.clear();
      lcd.print("Inserido: ");
      lcd.setCursor(0,1);
      lcd.print(dinheiroInserido);
      lcd.print(" Reais ");
      delay(1000);
    }
  }

  // Confirmar o valor inserido e calcular troco
  int troco = dinheiroInserido - valorNecessario; // Calcula o troco
  if (dinheiroInserido >= valorNecessario) {
    lcd.clear();
    lcd.print("Valor OK!");
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("Saldo Insuf.");
    delay(2000);
    return; // Retorna ao início
  }

int ciclosMoagem = 0;
  if (tecla != '4') {
    ciclosMoagem = selecionarCicloMoagem();
    if (ciclosMoagem == -1) {
      return; // Se o ciclo for inválido, retorna ao início
    }
  }

  // Verifica se tem água suficiente
  if (nivelAgua < consumo) {
    lcd.clear();
    lcd.print("Agua Insufic.");
    delay(2000);
    return; // Retorna ao início
  }

  // Consumir a quantidade de água necessária
  nivelAgua -= consumo;

    // Preparar o café ou apenas água quente, se todas as condições estiverem ok
  if (prepararCafe(ciclosMoagem, tecla)) {
  	lcd.clear();
 	lcd.print("Cafe Pronto!");
  	delay(3000);
  }
  
  // Mostrar o troco no final
  lcd.clear();
  lcd.print("Troco: ");
  lcd.print(troco); // Mostra o valor do troco
  delay(3000);
  

  lcd.clear();
  lcd.print("Agua Restante: ");
  lcd.setCursor(0,1);
  lcd.print(nivelAgua);
  lcd.print(" ml");
  delay(3000);

  lcd.clear();
  lcd.print("Obrigado");
  lcd.setCursor(0,1);
  lcd.print("Volte Sempre!");
  delay(3000);
}

int selecionarCicloMoagem() {
  lcd.clear();
  lcd.print("Selecionar Ciclo");
  lcd.setCursor(0,1);
  lcd.print("(1 a 8)");

  char tecla = 0;
  while (true) {
    tecla = teclado.getKey();
    if (tecla >= '1' && tecla <= '8') {
      int ciclos = tecla - '0';  // Converte o char para int
      lcd.clear();
      lcd.print("Ciclo");
      lcd.setCursor(0,1);
      lcd.print("Selecionado: ");
      lcd.print(ciclos);
      delay(1000);
      return ciclos;
    } else if (tecla) {
      lcd.clear();
      lcd.print("Ciclo Inválido");
      delay(1000);
      return -1; // Retorna -1 se o ciclo for inválido
    }
  }
}


bool verificarSensores() {
  // Verificar o status da temperatura
  int leituraTemperatura = analogRead(sensorTemperatura);
  float temperatura = leituraTemperatura * (5.0 / 1023.0) * 100.0; // Conversão para °C
  if (temperatura < tempIdeal) {
    lcd.clear();
    lcd.print("Temperatura");
    lcd.setCursor(0,1);
    lcd.print("Insuficiente");
    delay(2000);
    return false;
  }

  // Verificar o status da borra
  if (digitalRead(sensorBorra) == LOW) {
    lcd.clear();
    lcd.print("Borra Cheia!");
    delay(2000);
    return false;
  }

  // Verificar o nível de grãos
  int leituraGraos = analogRead(sensorGraos);
  if (leituraGraos < 130) {
    lcd.clear();
    lcd.print("Faltam Graos");
    delay(2000);
    return false;
  }

  // Verificar o nível de água
  if (nivelAgua < 5) { // Mínimo de água para funcionar
    lcd.clear();
    lcd.print("Falta Agua");
    delay(2000);
    return false;
  }

  // Se todos os sensores estão ok
  lcd.clear();
  lcd.print("Sensores OK");
  delay(1000); // Aguarda antes de prosseguir
  return true;
}

bool prepararCafe(int ciclosMoagem, char tipoCafe) {
  lcd.clear();
  lcd.print("Processando...");

  // Primeiro, ativa a bomba de água
  digitalWrite(motorAgua, HIGH); // Ativa a bomba de água
  delay(3000);                   // Tempo padrão para água
  digitalWrite(motorAgua, LOW);  // Desativa a bomba de água

  // Em seguida, realiza a moagem (se necessário)
  if (tipoCafe != '4') { // Se não for água
    for (int i = 0; i < ciclosMoagem; i++) {
      digitalWrite(motorMoagem, HIGH); // Ativa o motor de moagem
      delay(1000);                     // 1 segundo por ciclo de moagem
      digitalWrite(motorMoagem, LOW);  // Desativa o motor de moagem
    }
  }

  // Após a moagem, ligar o LED do café por 5 segundos e exibir a mensagem
  lcd.clear();
  lcd.print("Cafe Saindo!");
  digitalWrite(ledCafe, HIGH);
  delay(5000);
  digitalWrite(ledCafe, LOW);

  return true;
}
