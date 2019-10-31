#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above
#define led_verde       3
#define led_vermelho    2
#define servo           3
#define but_in          4
#define but_out         5

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

void ativar_servo() //true abre, false fecha
{
  //Ativa o servo e destrava/trava a porta
  if(!estado_porta) //Está trancada
  { //Abre a porta aqui
    digitalWrite(led_verde, HIGH);
    digitalWrite(led_vermelho, LOW);
    estado_porta = 1;
  }
  else
  { //Fecha a porta

    digitalWrite(led_verde, LOW);
    digitalWrite(led_vermelho, HIGH);
    estado_porta = 0;
  }
}


void detecta_membro(long nusp_lido) //Função prototipo
{
  bool abre = false;
  int i;
  for(i = 0; i <= indc; i++)
  {
    if(cadastro[i].nusp == nusp_lido)
    {
      ativar_servo(); 
      abre = true;
      break; //Saio do for neste instante
    } 
  }
  if(abre)
  { 
    Serial.println("\nBem vindo: ");
    Serial.println(cadastro[i].nome);
  }
  else
  {
    Serial.println("\n\nErro: Voce nao esta atutorizadx a entrar");
  }
  
}

void cadastra_membro(char nome[16], long nusp)
{
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

  detecta_membro(nusp_l);

  //----------------------------------------

  Serial.println(F("\n**Terminou**\n"));

  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  //Zerando as variaveis globais
  for(int j = 0; j < 16; j++)
  {
    nusp_char[j] = 32; //Colocando nulo em todas as posições dos vetores
    nome[j] = 32;
  }
}
//*****************************************************************************************//
