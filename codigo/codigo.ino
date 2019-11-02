#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above
#define led_verde       3
#define led_vermelho    2
#define servo           3
#define but_in          4 //Botão interno abre ou fecha!
#define but_out         5 //Botão externo vai apenas travar!!
#define NUMERO_MESTRE   42 //Número exclusivo do cartão mestre, cuja única finalidade é cadastrar novos membros
#define TEMP_MEMBRO     22 //Número que os cartões de membros "visitantes" terão - Esses não contarão com uma música personalizada

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

struct membro{
  char nome[16];
  long nusp; //int em arduinos com ATmega possui apenas 2 bytes, o que é insuficiente
};

char *nome = calloc(16, sizeof(char)); 
char *nusp_char = calloc(16,sizeof(char));

int estado_porta = 1; //0 para fechado, 1 para aberto.

int indc = -1; //Registra o índice dos cadastros
struct membro cadastro[5];

void ajusta_led() //Ajusta os leds  para o estado atual da porta
{
  if(estado_porta)
  {
    digitalWrite(led_verde, LOW);
    digitalWrite(led_vermelho, HIGH);
  }
  else
  {
    digitalWrite(led_verde, HIGH);
    digitalWrite(led_vermelho, LOW);
  }
  
}

void ativar_servo()
{ 
  //Adicionar reações do buzzer também
  //Ativa o servo e destrava/trava a porta
  if(!estado_porta) //Está trancada
  { //Abre a porta aqui
    estado_porta = 1;
    ajusta_led();
  }
  else
  { //Fecha a porta
    estado_porta = 0;
    ajusta_led();
  }
}

void cadastra_membro_temporario()
{
  Serial.println("Ola sr(a) admin, vamos cadastrar um membro temporario");
  delay(1500);
  int estado_led = HIGH;
  unsigned long pretempo = 0, tempo = millis();

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  while( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial())
  { //Enquanto ainda não aproximar o cartão
    if(tempo - pretempo >= 1000)
    {
      pretempo = tempo;
      digitalWrite(led_verde, estado_led);
      digitalWrite(led_vermelho, estado_led);
      estado_led = !estado_led; //Da próxima vez que entrar nesta função temporal, terá o efeito contrário
    }
  }
  //Saiu do while, significa que encontrou o cartão
  digitalWrite(led_verde, HIGH);
  digitalWrite(led_vermelho, HIGH);

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  
  
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  // Sobrenome - nusp visitante
  buffer[0] = '2';
  buffer[1] = '2';
     //TEMP_MEMBRO);
  //for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces

  block = 1;
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  // Primeiro nome 
  Serial.println(F("Type First name, ending with #"));
  strcpy(buffer,"aluno"); // read first name from serial
  //for (byte i = len; i < 20; i++) buffer[i] = ' ';     // pad with spaces

  block = 4;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 5;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));


  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  ajusta_led();
  delay(1000); //Dar um tempo até que o elemento desaproxime o cartão
}

void detecta_membro(long nusp_lido) //Função prototipo
{
  bool abre = false;
  int i;
  if(nusp_lido == NUMERO_MESTRE)
  {
    cadastra_membro_temporario();
    //Bipar o buzzer, indicando que deve aproximar o novo cartão a ser cadastrado
    return;
  } 
  
  for(i = 0; i <= indc; i++)
  { //Percorre todos os cadastros pesquisando por membros ou por cartões visitante
    if(cadastro[i].nusp == nusp_lido || nusp_lido == TEMP_MEMBRO)
    {
      ativar_servo(); 
      abre = true;
      break; //Saio do for neste instante
    } 
  }
  
  if(abre)
  { 
    Serial.println("\nBem vindo: ");
    //Serial.println(cadastro[i].nome); apresentação de nome meramente para testes
  }
  else
  {
    Serial.println("\n\nErro: Voce nao esta atutorizadx a entrar");
  }
  
}

void cadastra_membro(char nome[16], long nusp)
{ //No futuro, esta função deve carregar os cadastros presentes no cartão SD
  indc++;
  strcpy(cadastro[indc].nome, nome);
  cadastro[indc].nusp = nusp;
}

//*****************************************************************************************//
void setup() 
{
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  //Depois de setar o servomotor
  
  ativar_servo();
  
  cadastra_membro("Alguem", 10228323L);
  cadastra_membro("Fulano", 8283712L);
  cadastra_membro("Marcia", 12345678L);

  Serial.println(F("Aproxime o seu cartão:"));    //shows in serial that it is ready to read
}

//*****************************************************************************************//
void loop() 
{

  if(digitalRead(but_in)) //Apertaram o botão interno, movimentar o sistema
    ativar_servo();

  if(digitalRead(but_out) && estado_porta == 1) //Se o botão externo foi pressionado e a porta estava aberta, fecha a porta
    ativar_servo();
    
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte block;
  byte len;
  MFRC522::StatusCode status;

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Cartão detectado:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
 
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32 && buffer1[i] != 0)
    {
      nome[i] = buffer1[i]; //Copia o nome
    }
  }
  
  Serial.write(nome);
  Serial.print(" ");

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  
  for (uint8_t i = 0; i < 16; i++)
  {
    if(buffer2[i] != 0)
      nusp_char[i] = buffer2[i];
  }
  
  long nusp_l = atol(nusp_char);
  Serial.print(nusp_l);

  //----------------------------------------

  Serial.println(F("\n**Terminou**\n"));

  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();


  //Tenho que esperar o RFID completar seu ciclo, para, se necessário, eu poder utilizá-lo no cadastro
   detecta_membro(nusp_l);
  //Zerando as variaveis globais
  for(int j = 0; j < 16; j++)
  {
    nusp_char[j] = 32; //Colocando nulo em todas as posições dos vetores
    nome[j] = 32;
  }
}
//*****************************************************************************************//
