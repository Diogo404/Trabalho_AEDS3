#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <locale.h>

#define MAX_LINE_LENGTH 1024
const int endian = 1;

//Função para verificar o Endianness do sistema
bool is_bigendian(){
    char *y = (char*)&endian;
    return y;
}

//Função para tratar o Endianness do inteiro
int reverseInt (int intRegistro) {
    unsigned char c1, c2, c3, c4;

    if (is_bigendian() == false) {
        return intRegistro;
    } else {
        c1 = intRegistro & 255;
        c2 = (intRegistro >> 8) & 255;
        c3 = (intRegistro >> 16) & 255;
        c4 = (intRegistro >> 24) & 255;

        return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
    }
}

struct Registro {
    //Struct com as informações do registro
    int id;
    char carro[255];
    int ano;
    int preco;
    char combustivel[255];
};

bool ValidaLapide(int posicao){
    //Confere se o registro foi excluído "true" ou não "false"
    FILE *BaseDados = fopen("BaseDados.db", "rb");
    unsigned char lapide;
    fseek(BaseDados, posicao, SEEK_SET);
    size_t bytesRead = fread(&lapide, sizeof(lapide), 1, BaseDados);
    if(lapide == 0x01){
        return true;
    }
    return false;
}

//Realiza a carga inicial do arquivo CSV para o arquivo db
int RealizarCarga(char filename[MAX_LINE_LENGTH]){
    FILE *ArquivoCsv = fopen(filename, "r"); // Abre o arquivo CSV para leitura
    char line[MAX_LINE_LENGTH], carro[MAX_LINE_LENGTH], combustivel[MAX_LINE_LENGTH], *token;
    FILE *BaseDados = fopen("BaseDados.db", "wb"); // Abre o arquivo BaseDados para escrita
    unsigned char lapide = 0x00, RegistroBytes[sizeof(char)];
    uint32_t IntBytes;
    int tamanho, id, ano, preco, qtdCombustivel = 1, UltimoID = 0;

    // Verifica se os arquivos foram abertos corretamente
    if (ArquivoCsv == NULL || BaseDados == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    // Conta a quantidade de registros no arquivo CSV
    while (fgets(line, sizeof(line), ArquivoCsv)) {
        UltimoID++;
    }

    // Volta o ponteiro do arquivo para o início
    fseek(ArquivoCsv, 0, SEEK_SET);

    //Resolve o problema relacionado ao Endianness do processador
    UltimoID = reverseInt(UltimoID);

    // Copia os bytes do inteiro UltimoID para o array IntBytes
    memcpy(&IntBytes, &UltimoID, sizeof(int));

    //Escreve os bytes do ultimoId no arquivo BaseDados
    fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

    // Lê cada linha do arquivo CSV e escreve no arquivo BaseDados
    while (fgets(line, sizeof(line), ArquivoCsv)) {
        // Remove o caractere de nova linha do final da linha lida
        for(int i = 0; i<strlen(line); i++){
            if(line[i] == '\n'){
                line[i] = '\0';
            }
        }

        // Divide a linha em tokens usando a vírgula como delimitador e armazena os valores nas variáveis correspondentes
        token = strtok(line, ",");
            id = atoi(token);
        token = strtok(NULL, ",");
            strcpy(carro, token);  
        token = strtok(NULL, ",");
            ano = atoi(token);
        token = strtok(NULL, ",");
            preco = atoi(token);
        token = strtok(NULL, ",");
            strcpy(combustivel, token);

        // Verifica se o veículo tem mais de um combustível
        for(int i = 0; i<strlen(combustivel); i++){
            if(combustivel[i] == '/'){
                qtdCombustivel++;
            }
        }

        //Insere a lapide
        fwrite(&lapide, sizeof(lapide), 1, BaseDados);

        //Tamanho do registro = id + qtdCarro + carro + ano + qtdCombustivel + tamanho do combustivel + combustivel + preco
        tamanho = sizeof(int) + sizeof(int) + strlen(carro) + sizeof(int) + sizeof(int) + (sizeof(int) * qtdCombustivel) + (strlen(combustivel) - (qtdCombustivel - 1)) + sizeof(int);

        //Insere o tamanho do registro
        tamanho = reverseInt(tamanho);
        memcpy(&IntBytes, &tamanho, sizeof(int));
        fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

        //Insere o id
        id = reverseInt(id);
        memcpy(&IntBytes, &id, sizeof(int));
        fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

        //Insere o tamanho do carro
        tamanho = strlen(carro);
        tamanho = reverseInt(tamanho);
        memcpy(&IntBytes, &tamanho, sizeof(int));
        fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

        //Insere o carro
        for (int i = 0; i < strlen(carro); i++) {
            memcpy(RegistroBytes, &carro[i], sizeof(char));
            fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
        }

        //Insere o ano
        ano = reverseInt(ano);
        memcpy(&IntBytes, &ano, sizeof(int));
        fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

        //Insere a quantidade de combustiveis
        qtdCombustivel = reverseInt(qtdCombustivel);
        memcpy(&IntBytes, &qtdCombustivel, sizeof(int));
        fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

        //Estrutura de repetição para inserir os combustiveis
        for(int i = 0, j = 0; i < strlen(combustivel); i++){
            if(combustivel[i] == '/' || (i + 1) == strlen(combustivel)){
                tamanho = i + 1;
                //Insere o tamanho do combustivel
                tamanho = reverseInt(tamanho);
                memcpy(&IntBytes, &tamanho, sizeof(int));
                fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);

                //Insere o combustivel
                for (; j <= i && combustivel[j] == '/'; j++) {
                    memcpy(RegistroBytes, &combustivel[j], sizeof(char));
                    fwrite(RegistroBytes, sizeof(RegistroBytes), 1, BaseDados);
                }
            }
        }

        //Insere o preço
        preco = reverseInt(preco);
        memcpy(&IntBytes, &preco, sizeof(int));
        fwrite(&IntBytes, sizeof(IntBytes), 1, BaseDados);
    }

    fclose(ArquivoCsv);
    fclose(BaseDados);
    return EXIT_SUCCESS;
}

//Insere um novo registro no arquivo BaseDados
void InserirRegistro(struct Registro carroRegistro){
    FILE *baseDados = fopen("BaseDados.db", "rb+");
    unsigned char lapide = 0x00, registroBytes[sizeof(char)];
    uint32_t intBytes;
    int ultimoId, tamanho, qtdCombustivel = 1;

    //Le o último ID do arquivo BaseDados e incrementa para o novo registro
    fread(&intBytes, sizeof(uint8_t), 4, baseDados);
    memcpy(&ultimoId, &intBytes, sizeof(int));
    ultimoId++;
    carroRegistro.id = ultimoId;

    //Atualiza o último ID no arquivo BaseDados
    fseek(baseDados, 0, SEEK_SET);
    ultimoId = reverseInt(ultimoId);
    memcpy(&intBytes, &ultimoId, sizeof(int));
    fwrite(&intBytes, sizeof(uint8_t), 1, baseDados);

    // Mover o ponteiro do arquivo para o final para adicionar o novo registro
    fseek(baseDados, 0, SEEK_END);

    //Insere a lapide
    fwrite(&lapide, sizeof(lapide), 1, baseDados);

    // Verifica se o veículo tem mais de um combustível
    for(int i = 0; i<strlen(carroRegistro.combustivel); i++){
        if(carroRegistro.combustivel[i] == '/'){
            qtdCombustivel++;
        }
    }

    //Tamanho do registro = id + qtdCarro + carro + ano + qtdCombustivel + combustivel + preco
    tamanho = sizeof(int) + sizeof(int) + strlen(carroRegistro.carro) + sizeof(int) + sizeof(int) + (sizeof(int) * qtdCombustivel) + (strlen(carroRegistro.combustivel) - (qtdCombustivel - 1)) + sizeof(int);
    tamanho = reverseInt(tamanho);
    memcpy(&intBytes, &tamanho, sizeof(int));
    fwrite(&intBytes, sizeof(intBytes), 1, baseDados);
    
    //Insere o id
    carroRegistro.id = reverseInt(carroRegistro.id);
    memcpy(&intBytes, &carroRegistro.id, sizeof(int));

    //Insere o tamanho do carro
    tamanho = strlen(carroRegistro.carro);
    tamanho = reverseInt(tamanho);
    memcpy(&intBytes, &tamanho, sizeof(int));
    fwrite(&intBytes, sizeof(intBytes), 1, baseDados);

    //Insere o carro
    for (int i = 0; i < strlen(carroRegistro.carro); i++) {
        memcpy(registroBytes, &carroRegistro.carro[i], sizeof(char));
        fwrite(registroBytes, sizeof(registroBytes), 1, baseDados);
    }

    //Insere o ano
    carroRegistro.ano = reverseInt(carroRegistro.ano);
    memcpy(&intBytes, &carroRegistro.ano, sizeof(int));
    fwrite(&intBytes, sizeof(intBytes), 1, baseDados);

    //Estrutura de repetição para inserir os combustiveis
    for(int i = 0, j = 0; i < strlen(carroRegistro.combustivel); i++){
        if(carroRegistro.combustivel[i] == '/' || (i + 1) == strlen(carroRegistro.combustivel)){
            tamanho = i + 1;
            //Insere o tamanho do combustivel
            tamanho = reverseInt(tamanho);
            memcpy(&intBytes, &tamanho, sizeof(int));
            fwrite(&intBytes, sizeof(intBytes), 1, baseDados);

            //Insere o combustivel
            for (; j <= i && carroRegistro.combustivel[j] == '/'; j++) {
                memcpy(registroBytes, &carroRegistro.combustivel[j], sizeof(char));
                fwrite(registroBytes, sizeof(registroBytes), 1, baseDados);                
            }
        }
    }

    //Insere o preco
    carroRegistro.preco = reverseInt(carroRegistro.preco);
    memcpy(&intBytes, &carroRegistro.preco, sizeof(int));
    fwrite(&intBytes, sizeof(intBytes), 1, baseDados);

    fclose(baseDados);
}

int BuscaRegistro(int id){
    FILE *BaseDados = fopen("BaseDados.db", "rb");
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
    FILE *BaseDados = fopen("BaseDados.db", "rb");
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
        memcpy(&carroRegistro.combustivel, RegistroBytes, Tamanho);

        bytesRead = fread(IntBytes, sizeof(unsigned char), 4, BaseDados);
        for (int i = 0; i < sizeof(InverteIntBytes); i++) {
            InverteIntBytes[i] = IntBytes[3 - i];
        }
        memcpy(&carroRegistro.preco, InverteIntBytes, sizeof(int));
        printf("ID: %d\n", carroRegistro.id);
        printf("Carro: %s\n", carroRegistro.carro);
        printf("Ano: %d\n", carroRegistro.ano);
        printf("Km: %s\n", carroRegistro.combustivel);
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
    if(strlen(carroRegistro->combustivel) < TamanhoKm){
        for(int i = strlen(carroRegistro->combustivel); i<TamanhoKm; i++){
            carroRegistro->combustivel[i] = ' ';
        }
    }
    if(TamanhoCarro < strlen(carroRegistro->carro) || TamanhoKm < strlen(carroRegistro->combustivel)){
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
        Tamanho = sizeof(int) + sizeof(int) + strlen(carroRegistro.carro) + sizeof(int) + sizeof(int) + strlen(carroRegistro.combustivel) + sizeof(int);
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
        Tamanho = strlen(carroRegistro.combustivel);
        memcpy(IntBytes, &Tamanho, sizeof(int));
        for (int i = 0; i < sizeof(int); i++) {
            InverteIntBytes[i] = IntBytes[sizeof(int) - 1 - i];
        }
        fwrite(InverteIntBytes, sizeof(InverteIntBytes), 1, BaseDados);
        //Insere o km
        for (int i = 0; i < strlen(carroRegistro.combustivel); i++) {
            memcpy(RegistroBytes, &carroRegistro.combustivel[i], sizeof(char));
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
        memcpy(RegistroBytes, &carroRegistro.combustivel[i], sizeof(char));
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
    char filename[MAX_LINE_LENGTH], ano[255];
    int opt = 1, BuscarId, LerIdDe, LerIdAte;
    struct Registro carroRegistro;

    printf("Digite o nome do arquivo csv com a base de dados: \n");
    fgets(filename, sizeof(filename), stdin);
    fflush(stdin);
    filename[strcspn(filename, "\n")] = 0; 
    RealizarCarga(filename);

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
                printf("Digite o combustível do carro: \n");
                scanf("%s", &carroRegistro.combustivel);
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
                printf("Digite o combustível do carro: \n");
                scanf("%s", &carroRegistro.combustivel);
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