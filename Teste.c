#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define MAX_LINE_LENGTH 1024


int ContaRegistros(char filename[]){
    FILE *ArquivoCsv = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int UltimoID = 0;

    if (ArquivoCsv == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), ArquivoCsv)) {
        //Conta as linhas do arquivo e encontra o ultimo ID. (Ultimo ID = Quantidade de linhas)
        UltimoID++;
    }

    fclose(ArquivoCsv);
    return UltimoID;
}

int RealizarCarga(char filename[MAX_LINE_LENGTH], int UltimoID){
    FILE *ArquivoCsv = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    FILE *BaseDados = fopen("BaseDados.txt", "w");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)],  lapide = 0x00, RegistroBytes[sizeof(char)];
    int LineLength;

    if (ArquivoCsv == NULL || BaseDados == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    // Copia os bytes do inteiro UltimoID para o array IntBytes
    memcpy(IntBytes, &UltimoID, sizeof(int));
    // Inverter a ordem dos bytes
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    // Escrever os bytes invertidos no arquivo BaseDados
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);

    while (fgets(line, sizeof(line), ArquivoCsv)) {

        for(int i = 0; i<strlen(line); i++){
            if(line[i] == ','){
                line[i] = '|';
            }else if(line[i] == '\n'){
                line[i] = ';';
            }
        }
        //Insere a lapide
        fwrite(&lapide, sizeof(lapide), 1, BaseDados);
        //Insere o tamanho do registro
        LineLength = strlen(line);
        //printf("Tamanho do registro: %d\n", LineLength);
        memcpy(IntBytes, &LineLength, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o registro
        for (int i = 0; i < LineLength; i++) {
            memcpy(RegistroBytes, &line[i], sizeof(char));
            fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
        }
    }

    fclose(ArquivoCsv);
    fclose(BaseDados);
    return EXIT_SUCCESS;
}

void InserirRegistro(char registro[MAX_LINE_LENGTH]){
    FILE *BaseDados = fopen("BaseDados.txt", "r");
    int UltimoID;
    if (BaseDados == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }
    unsigned char buffer[4];
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 4, BaseDados);
    // Inverter a ordem dos bytes lidos
    unsigned char InvertedBuffer[4];
    for (int i = 0; i < 4; i++) {
        InvertedBuffer[i] = buffer[3 - i];
    }

    memcpy(&UltimoID, InvertedBuffer, sizeof(int));
    UltimoID++;
    fclose(BaseDados);
    // Mover o ponteiro do arquivo para o final para adicionar o novo registro
    //fseek(BaseDados, 0, SEEK_END);
    //fwrite(registro, sizeof(char), strlen(registro), BaseDados);
}

int main() {
    setlocale(LC_ALL, "");
    char filename[MAX_LINE_LENGTH];  // Nome do arquivo CSV
    int opt = 1, BuscarId, LerIdDe, LerIdAte;
    char carro[50], registro[MAX_LINE_LENGTH];
    int ano, km, preco;

    printf("Digite o nome do arquivo csv com a base de dados: \n");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0; 
    RealizarCarga(filename, ContaRegistros(filename));

    while(opt != 0){
        printf("Selecione a opção: \n");
        printf("1 - Inserir novo registro: \n");
        printf("2 - Ler um registro: \n");
        printf("3 - Ler vários registros: \n");
        printf("4 - Atualizar um registro: \n");
        printf("5 - Deletar um registro: \n");
        printf("0 - Sair \n");
        scanf("%d", &opt);
        switch (opt){
            case 0:
                printf("Saindo...\n");
                break;
            case 1:
                printf("Digite o nome do carro: \n");
                scanf("%s", carro);
                printf("Digite o ano do carro: \n");
                scanf("%i", &ano);
                printf("Digite o km do carro: \n");
                scanf("%i", &km);
                printf("Digite o preco do carro: \n");
                scanf("%i", &preco);
                snprintf(registro, sizeof(registro), "|%s|%d|%d|%d;", carro, ano, km, preco);
                InserirRegistro(registro);
                break;
            case 2:
                printf("Digite o ID do registro que deseja ler: \n");
                scanf("%d", &BuscarId);
                break;
            case 3:
                printf("Digite o primeiro ID do registro que deseja ler: \n");
                scanf("%d", &LerIdDe);
                printf("Digite o último ID do registro que deseja ler: \n");
                scanf("%d", &LerIdAte);
                break;
            case 4:
                printf("Digite o ID do registro que deseja atualizar: \n");
                scanf("%d", &BuscarId);
                break;
            case 5:
                printf("Digite o ID do registro que deseja deletar: \n");
                scanf("%d", &BuscarId);
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }
}