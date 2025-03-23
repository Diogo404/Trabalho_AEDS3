#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

#define MAX_LINE_LENGTH 1024

struct Registro {
    //Struct com as informações do registro
    int id;
    char carro[255];
    int ano;
    char km[255];
    int preco;
};

bool ValidaLapide(int posicao){
    //Confere se o registro foi excluído "true" ou não "false"
    FILE *BaseDados = fopen("BaseDados.hex", "rb");
    unsigned char lapide;
    fseek(BaseDados, posicao, SEEK_SET);
    size_t bytesRead = fread(&lapide, sizeof(lapide), 1, BaseDados);
    if(lapide == 0x01){
        return true;
    }
    return false;
}

int ContaRegistros(char filename[]){
    //Conta as linhas do arquivo e encontra o ultimo ID. Usado apenas na carga inicial
    FILE *ArquivoCsv = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int UltimoID = 0;

    if (ArquivoCsv == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), ArquivoCsv)) {
        UltimoID++;
    }

    fclose(ArquivoCsv);
    return UltimoID;
}

int RealizarCarga(char filename[MAX_LINE_LENGTH], int UltimoID){
    //Realiza a carga inicial do arquivo CSV para o arquivo binário
    FILE *ArquivoCsv = fopen(filename, "r");
    char line[MAX_LINE_LENGTH], carro[255], km[255];
    FILE *BaseDados = fopen("BaseDados.hex", "wb");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)],  lapide = 0x00, RegistroBytes[sizeof(char)];
    int Tamanho, ano, preco, id, teste = 0;

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
            if(line[i] == '\n'){
                line[i] = '\0';
            }
        }
        char *token = strtok(line, ",");
        id = atoi(token);
        token = strtok(NULL, ",");
        strcpy(carro, token);    
        token = strtok(NULL, ",");
        ano = atoi(token);
        token = strtok(NULL, ",");
        strcpy(km, token);
        token = strtok(NULL, ",");
        preco = atoi(token);
        //Insere a lapide
        fwrite(&lapide, sizeof(lapide), 1, BaseDados);
        //Insere o tamanho do registro
        Tamanho = sizeof(int) + sizeof(int) + strlen(carro) + sizeof(int) + sizeof(int) + strlen(km) + sizeof(int);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o id
        memcpy(IntBytes, &id, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o tamanho do carro
        Tamanho = strlen(carro);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o carro
        for (int i = 0; i < strlen(carro); i++) {
            memcpy(RegistroBytes, &carro[i], sizeof(char));
            fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
        }
        //Insere o ano
        memcpy(IntBytes, &ano, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o tamanho do km
        Tamanho = strlen(km);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o km
        for (int i = 0; i < strlen(km); i++) {
            memcpy(RegistroBytes, &km[i], sizeof(char));
            fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
        }
        //Insere o preco
        memcpy(IntBytes, &preco, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    }

    fclose(ArquivoCsv);
    fclose(BaseDados);
    return EXIT_SUCCESS;
}

void InserirRegistro(struct Registro carroRegistro){
    FILE *BaseDados = fopen("BaseDados.hex", "rb+");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)], lapide = 0x00, RegistroBytes[sizeof(char)];
    int UltimoID, Tamanho;
    size_t bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
    
    // Inverter a ordem dos bytes lidos
    for (int i = 0; i < sizeof(InverteIntBytes); i++) {
        InverteIntBytes[i] = IntBytes[3 - i];
    }
    memcpy(&UltimoID, InverteIntBytes, sizeof(int));
    UltimoID++;
    carroRegistro.id = UltimoID;

    fseek(BaseDados, 0, SEEK_SET);

    memcpy(IntBytes, &UltimoID, sizeof(int));
    // Inverter a ordem dos bytes
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    // Escrever os bytes invertidos no arquivo BaseDados
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);

    // Mover o ponteiro do arquivo para o final para adicionar o novo registro
    fseek(BaseDados, 0, SEEK_END);
    //Insere a lapide
    fwrite(&lapide, sizeof(lapide), 1, BaseDados);

    //Insere o tamanho do registro
    Tamanho = sizeof(int) + sizeof(int) + strlen(carroRegistro.carro) + sizeof(int) + sizeof(int) + strlen(carroRegistro.km) + sizeof(int);
    memcpy(IntBytes, &Tamanho, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    //Insere o id
    memcpy(IntBytes, &carroRegistro.id, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    //Insere o tamanho do carro
    Tamanho = strlen(carroRegistro.carro);
    memcpy(IntBytes, &Tamanho, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    //Insere o carro
    for (int i = 0; i < strlen(carroRegistro.carro); i++) {
        memcpy(RegistroBytes, &carroRegistro.carro[i], sizeof(char));
        fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
    }
    //Insere o ano
    memcpy(IntBytes, &carroRegistro.ano, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    //Insere o tamanho do km
    Tamanho = strlen(carroRegistro.km);
    memcpy(IntBytes, &Tamanho, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    //Insere o km
    for (int i = 0; i < strlen(carroRegistro.km); i++) {
        memcpy(RegistroBytes, &carroRegistro.km[i], sizeof(char));
        fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
    }
    //Insere o preco
    memcpy(IntBytes, &carroRegistro.preco, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);

    fclose(BaseDados);
}

int BuscaRegistro(int id){
    FILE *BaseDados = fopen("BaseDados.hex", "rb");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)], lapide = 0x00, RegistroBytes[sizeof(char)];
    int idRegistro = 0, UltimoID, Tamanho, posicao;

    size_t bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
    for (int i = 0; i < sizeof(InverteIntBytes); i++) {
        InverteIntBytes[i] = IntBytes[3 - i];
    }
    posicao = 4;
    memcpy(&UltimoID, InverteIntBytes, sizeof(int));
    while (idRegistro != UltimoID && idRegistro != id){
        bytesRead = fread(&lapide, sizeof(lapide), 1, BaseDados);
        bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        posicao += 5;
        memcpy(&Tamanho, InverteIntBytes, sizeof(int));
        bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&idRegistro, InverteIntBytes, sizeof(int));
        if(idRegistro == id){
            return posicao;
        }else{
            fseek(BaseDados, Tamanho-4, SEEK_CUR);
            posicao += Tamanho;
        }
    }
}

void MostrarRegistro(int posicao){
    FILE *BaseDados = fopen("BaseDados.hex", "rb");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)], lapide = 0x00, RegistroBytes[255];
    struct Registro carroRegistro;
    int Tamanho;
    if(ValidaLapide(posicao-5)){
    }else{
        fseek(BaseDados, posicao, SEEK_CUR);

        size_t bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&carroRegistro.id, InverteIntBytes, sizeof(int));

        fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&Tamanho, InverteIntBytes, sizeof(int));

        bytesRead = fread(RegistroBytes, sizeof(unsigned char), Tamanho, BaseDados);
        memcpy(&carroRegistro.carro, RegistroBytes, Tamanho);

        bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&carroRegistro.ano, InverteIntBytes, sizeof(int));

        bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&Tamanho, InverteIntBytes, sizeof(int));

        bytesRead = fread(RegistroBytes, sizeof(unsigned char), Tamanho, BaseDados);
        memcpy(&carroRegistro.km, RegistroBytes, Tamanho);

        bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&carroRegistro.preco, InverteIntBytes, sizeof(int));
        printf("ID: %d\n", carroRegistro.id);
        printf("Carro: %s\n", carroRegistro.carro);
        printf("Ano: %d\n", carroRegistro.ano);
        printf("Km: %s\n", carroRegistro.km);
        printf("Preco: %d\n", carroRegistro.preco);
    }
    fclose(BaseDados);
}

bool RegistroExcedeChar(int posicao, struct Registro *carroRegistro){
    FILE *BaseDados = fopen("BaseDados.hex", "rb");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)], lapide = 0x00, RegistroBytes[sizeof(char)];
    int TamanhoCarro, TamanhoKm;

    fseek(BaseDados, posicao+4, SEEK_SET);
    fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
    for (int i = 0; i < sizeof(InverteIntBytes); i++) {
        InverteIntBytes[i] = IntBytes[3 - i];
    }
    memcpy(&TamanhoCarro, InverteIntBytes, sizeof(int));
    
    fseek(BaseDados, TamanhoCarro+4, SEEK_CUR);
    fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
    for (int i = 0; i < sizeof(InverteIntBytes); i++) {
        InverteIntBytes[i] = IntBytes[3 - i];
    }
    memcpy(&TamanhoKm, InverteIntBytes, sizeof(int));
    fclose(BaseDados);
    if(strlen(carroRegistro->carro) < TamanhoCarro){
        for(int i = strlen(carroRegistro->carro); i<TamanhoCarro; i++){
            carroRegistro->carro[i] = ' ';
        }
    }
    if(strlen(carroRegistro->km) < TamanhoKm){
        for(int i = strlen(carroRegistro->km); i<TamanhoKm; i++){
            carroRegistro->km[i] = ' ';
        }
    }
    if(TamanhoCarro < strlen(carroRegistro->carro) || TamanhoKm < strlen(carroRegistro->km)){
        return true;
    }
    return false;
}

void ApagarRegistro(int posicao){
    FILE *BaseDados = fopen("BaseDados.hex", "rb+");
    unsigned char lapide = 0x01;
    fseek(BaseDados, posicao-5, SEEK_SET);
    fwrite(&lapide, sizeof(lapide), 1, BaseDados);
    fclose(BaseDados);
}

void AtualizarRegistro(int posicao, struct Registro carroRegistro){
    FILE *BaseDados = fopen("BaseDados.hex", "rb+");
    unsigned char IntBytes[sizeof(int)], InverteIntBytes[sizeof(int)], lapide = 0x00, RegistroBytes[sizeof(char)];
    int Tamanho;

    fseek(BaseDados, posicao, SEEK_SET);
    fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
    for (int i = 0; i < sizeof(InverteIntBytes); i++) {
        InverteIntBytes[i] = IntBytes[3 - i];
    }
    memcpy(&carroRegistro.id, InverteIntBytes, sizeof(int));

    if(RegistroExcedeChar(posicao, &carroRegistro)){
        ApagarRegistro(posicao);
        fseek(BaseDados, 0, SEEK_END);
        //Insere a lapide
        fwrite(&lapide, sizeof(lapide), 1, BaseDados);
        //Insere o tamanho do registro
        Tamanho = sizeof(int) + sizeof(int) + strlen(carroRegistro.carro) + sizeof(int) + sizeof(int) + strlen(carroRegistro.km) + sizeof(int);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o id
        memcpy(IntBytes, &carroRegistro.id, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o tamanho do carro
        Tamanho = strlen(carroRegistro.carro);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o carro
        for (int i = 0; i < strlen(carroRegistro.carro); i++) {
            memcpy(RegistroBytes, &carroRegistro.carro[i], sizeof(char));
            fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
        }
        //Insere o ano
        memcpy(IntBytes, &carroRegistro.ano, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o tamanho do km
        Tamanho = strlen(carroRegistro.km);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o km
        for (int i = 0; i < strlen(carroRegistro.km); i++) {
            memcpy(RegistroBytes, &carroRegistro.km[i], sizeof(char));
            fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
        }
        //Insere o preco
        memcpy(IntBytes, &carroRegistro.preco, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);

        fclose(BaseDados);
    }else{
    //Insere o carro
    for (int i = 0; i < Tamanho; i++) {
        memcpy(RegistroBytes, &carroRegistro.carro[i], sizeof(char));
        fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
    }
    //Insere o ano
    memcpy(IntBytes, &carroRegistro.ano, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    //Pula o tamanho do Km
    fseek(BaseDados, 4, SEEK_CUR);
    //Insere o km
    for (int i = 0; i < Tamanho; i++) {
        memcpy(RegistroBytes, &carroRegistro.km[i], sizeof(char));
        fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
    }
    //Insere o preco
    memcpy(IntBytes, &carroRegistro.preco, sizeof(int));
    for (int i = 0; i < sizeof(int); i++) {
        InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
    }
    fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
    fclose(BaseDados);
    }
}


int main() {
    setlocale(LC_ALL, "");
    char filename[MAX_LINE_LENGTH], ano[255];  // Nome do arquivo CSV
    int opt = 1, BuscarId, LerIdDe, LerIdAte;
    struct Registro carroRegistro;

    printf("Digite o nome do arquivo csv com a base de dados: \n");
    fgets(filename, sizeof(filename), stdin);
    fflush(stdin);
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
                scanf("%s", &carroRegistro.carro);
                fflush(stdin);
                printf("Digite o ano do carro: \n");
                scanf("%s", &ano);
                strcat(ano, "0101");
                carroRegistro.ano = atoi(ano);
                fflush(stdin);
                printf("Digite o km do carro: \n");
                scanf("%s", &carroRegistro.km);
                fflush(stdin);
                printf("Digite o preco do carro: \n");
                scanf("%d", &carroRegistro.preco);
                fflush(stdin);

                InserirRegistro(carroRegistro);
                break;
            case 2:
                printf("Digite o ID do registro que deseja ler: \n");
                scanf("%d", &BuscarId);
                MostrarRegistro(BuscaRegistro(BuscarId));
                break;
            case 3:
                printf("Digite o primeiro ID do registro que deseja ler: \n");
                scanf("%d", &LerIdDe);
                printf("Digite o último ID do registro que deseja ler: \n");
                scanf("%d", &LerIdAte);
                for(int i = LerIdDe; i<=LerIdAte; i++){
                    MostrarRegistro(BuscaRegistro(i));
                }
                break;
            case 4:
                printf("Digite o ID do registro que deseja atualizar: \n");
                scanf("%d", &BuscarId);
                printf("Digite o nome do carro: \n");
                scanf("%s", &carroRegistro.carro);
                fflush(stdin);
                printf("Digite o ano do carro: \n");
                scanf("%s", &ano);
                strcat(ano, "0101");
                carroRegistro.ano = atoi(ano);
                fflush(stdin);
                printf("Digite o km do carro: \n");
                scanf("%s", &carroRegistro.km);
                fflush(stdin);
                printf("Digite o preco do carro: \n");
                scanf("%d", &carroRegistro.preco);
                fflush(stdin);
                AtualizarRegistro(BuscaRegistro(BuscarId), carroRegistro);
                break;
            case 5:
                printf("Digite o ID do registro que deseja deletar: \n");
                scanf("%d", &BuscarId);
                ApagarRegistro(BuscaRegistro(BuscarId));
                printf("Registro deletado com sucesso\n");
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }
}