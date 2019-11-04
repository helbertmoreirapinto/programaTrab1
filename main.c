#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define TAM_REG 85
#define TAM_CAB 19
#define TAM_CAMPO_FIXO 8
#define TAM_COMANDO 100

#define OK 0
#define ERRO 1
#define STATUS_OK '1'
#define TODOS -1

#define UF_ORIG "estadoOrigem"
#define UF_DEST "estadoDestino"
#define CIDADE_ORIG "cidadeOrigem"
#define CIDADE_DEST "cidadeDestino"
#define DISTANCIA "distancia"
#define TEMPO "tempoViagem"

#define TRANSFER            1
#define READ_ALL            2
#define READ_FIL            3
#define READ_RRN            4
#define DELETE_STATIC_FIL   5
#define INCLUDE_STATIC      6
#define REALSE_RRN          7
#define COMPACT             8

typedef struct {
    char UF_orig[3];
    char UF_dest[3];
    int distancia;
    char cidade_orig[20];
    char cidade_dest[20];
    char tempo[12];
} Registro;
typedef Registro* Registro_PTR;
typedef Registro** Registro_PTR_PTR;


typedef struct {
    char status;
    int numeroVertices;
    int numeroArestas;
    char dataUltimaCompactacao[10];
} Cabecalho;
typedef Cabecalho* Cabecalho_PTR;


typedef struct {
    char nome_vertice[20];
    int qtd_rep;
} Vertice;
typedef Vertice* Vertice_PTR;
typedef Vertice** Vertice_PTR_PTR;


typedef struct {
    Vertice_PTR_PTR lista_vertice;
    int numeroVertices;
    int numeroArestas;
} ListaArestaVertice;
typedef ListaArestaVertice* ListaArestaVertice_PTR;


/* Prototipos de funcoes */
void transfere_dados_csv_bin(char*, char*);
Registro_PTR ler_reg_csv(char, FILE*);
int salvar_registro(FILE*, Registro_PTR);
void limpar_memoria(Registro_PTR);
void atualizar_arq_vertice(FILE*, ListaArestaVertice_PTR);
char* get_data_sistema_formatada();
void exibir_bin(char*, int);
void exibe_reg(int, Registro_PTR);
int ler_reg(FILE*, Registro_PTR);
void compact(char*, char*);
void atualizar_campo_registro(char*, int);
void inserir_aresta(ListaArestaVertice_PTR, char*);

int main() {
    int operacao;
    int param;
    char param_1[50];
    char param_2[50];
    char param_3[50];

    fscanf(stdin, "%d", &operacao);

    switch(operacao) {
    case TRANSFER:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%s", param_2);
        fgetc(stdin);

        transfere_dados_csv_bin(param_1, param_2);
        break;

    case READ_ALL:
        fscanf(stdin, "%s", param_1);
        fgetc(stdin);

        exibir_bin(param_1, TODOS);
        break;

    case READ_FIL:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%s", param_2);
        fscanf(stdin, "%s", param_3);
        fgetc(stdin);

        break;

    case READ_RRN:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        exibir_bin(param_1, param);
        break;

    case DELETE_STATIC_FIL:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        break;

    case INCLUDE_STATIC:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        break;

    case REALSE_RRN:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        atualizar_campo_registro(param_1, param);
        break;

    case COMPACT:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%s", param_2);
        fgetc(stdin);

        compact(param_1, param_2);
        break;

    default:
        printf("ERRO NO OPERADOR COMANDO\n");
    }

    return 0;
}

void transfere_dados_csv_bin(char* nome_csv, char* nome_bin) {
    FILE* file_csv = fopen(nome_csv, "r");
    FILE* file_bin = fopen(nome_bin, "wb+");
    if(!file_csv || !file_bin) {
        printf("ERRO");
        return;
    }
    ListaArestaVertice_PTR lista;
    Cabecalho_PTR cab;
    Registro_PTR reg;
    char* data;

    //Pulando cabecalho arquivo .CSV
    fscanf(file_csv, "%*[^\n]");
    fgetc(file_csv);

    //Escrevendo cabecalho .BIN
    cab = calloc(1, sizeof(Cabecalho));
    fwrite(&cab->status, sizeof(char), 1, file_bin);
    fwrite(&cab->numeroVertices, sizeof(int), 1, file_bin);
    fwrite(&cab->numeroArestas, sizeof(int), 1, file_bin);
    fwrite(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

    //LE(.CSV) e SALVA(.BIN) um registro por vez
    char c;
    while((c = fgetc(file_csv)) != EOF) {
        reg = ler_reg_csv(c, file_csv);
        salvar_registro(file_bin, reg);
        free(reg);
    }

    //Contar Vertices e Arestas
    lista = calloc(1, sizeof(ListaArestaVertice));
    atualizar_arq_vertice(file_bin, lista);

    //Reescrever cabecalho
    rewind (file_bin);
    cab->status = STATUS_OK;
    cab->numeroVertices = lista->numeroVertices;
    cab->numeroArestas = lista->numeroArestas;
    data = get_data_sistema_formatada();
    strcpy(cab->dataUltimaCompactacao, data);
    free(data);
    fwrite(&cab->status, sizeof(char), 1, file_bin);
    fwrite(&cab->numeroVertices, sizeof(int), 1, file_bin);
    fwrite(&cab->numeroArestas, sizeof(int), 1, file_bin);
    fwrite(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

    fclose(file_csv);
    fclose(file_bin);
}

void atualizar_arq_vertice(FILE* file_bin, ListaArestaVertice_PTR lista) {
    FILE* file_vert = fopen("vertices.bin", "wb");
    if(!file_vert) {
        printf("Erro ao abrir files\n");
        return;
    }

    int tam_cidade_orig, tam_cidade_dest;
    char* campo_var;
    char* nome_cidade;
    int aux;
    char reg_rem;

    rewind(file_bin);
    fseek(file_bin, TAM_CAB, SEEK_CUR); //Pula cabecalho .BIN

    while(true) {
        reg_rem = fgetc(file_bin);
        if(reg_rem == '*')
            continue;

        fseek(file_bin, TAM_CAMPO_FIXO - 1, SEEK_CUR); //Pula os campos fixos - 1, pois ja esta lendo um caracter pra ver se foi removido
        campo_var = calloc(TAM_REG, sizeof(char));
        aux = fread(campo_var, (TAM_REG - TAM_CAMPO_FIXO)*sizeof(char), 1, file_bin); //Le todo conteudo de campo variavel
        if(!aux) //Se nao leu acabou arquivo
            break;

        lista->numeroArestas++;

        //Add cidade_orig aos vertices
        tam_cidade_orig = strcspn(&campo_var[0], "|");
        nome_cidade = (char*) calloc(tam_cidade_orig+1, sizeof(char));
        strncpy(nome_cidade, &campo_var[0], tam_cidade_orig);
        inserir_aresta(lista, nome_cidade);
        free(nome_cidade);

        //Add cidade_dest aos vertices
        tam_cidade_dest = strcspn(&campo_var[tam_cidade_orig+1], "|");
        nome_cidade = (char*) calloc(tam_cidade_dest+1, sizeof(char));
        strncpy(nome_cidade, &campo_var[tam_cidade_orig+1], tam_cidade_dest);
        inserir_aresta(lista, nome_cidade);
        free(nome_cidade);

        free(campo_var);
    }

    //Salva no arquivo os dados contidos na lista ordenada
    for(int i  = 0; i < lista->numeroVertices; i++) {
        fwrite(lista->lista_vertice[i]->nome_vertice, 20 * sizeof(char), 1, file_vert);
        fwrite(&lista->lista_vertice[i]->qtd_rep, sizeof(int), 1, file_vert);
    }

    fclose(file_vert);
}

void inserir_aresta(ListaArestaVertice_PTR lista, char* cidade) {
    Vertice_PTR novo;
    int tam = lista->numeroVertices;
    int aux_cmp;
    int inf = 0;
    int sup = tam-1;
    int meio;
    int ind = 0;
    bool ja_existe_na_lista = false;
    while (inf <= sup) {
        meio = (int)(inf + sup)/2;
        aux_cmp = strcmp(cidade, lista->lista_vertice[meio]->nome_vertice);
        if(aux_cmp < 0) {
            sup = meio - 1;
            ind = meio;
        } else if(aux_cmp > 0) {
            inf = meio+1;
            ind = inf;
        } else {
            lista->lista_vertice[meio]->qtd_rep++;
            ja_existe_na_lista = true;
            break;
        }
    }

    if(!ja_existe_na_lista) {
        novo = (Vertice_PTR) malloc(sizeof(Vertice));
        strcpy(novo->nome_vertice, cidade);
        novo->qtd_rep = 1;
        lista->lista_vertice = (Vertice_PTR_PTR) realloc(lista->lista_vertice, (tam+1) * sizeof(Vertice_PTR));
        lista->numeroVertices++;
        for(int i = tam; i > ind; i--)
            lista->lista_vertice[i] = lista->lista_vertice[i-1];
        lista->lista_vertice[ind] = novo;
    }
}

void atualizar_campo_registro(char* nome_file, int qtd_atualizacoes) {
    FILE* file_bin = fopen(nome_file, "ab+");
    if(!file_bin) {
        printf("ERRO");
        return;
    }
    int RRN;
    char campo[50];
    char valor[50];

    Registro_PTR reg;
    Cabecalho_PTR cab;
    int aux;
    //Escrevendo cabecalho .BIN
    rewind(file_bin);
    cab = calloc(1, sizeof(Cabecalho));
    fwrite(&cab->status, sizeof(char), 1, file_bin);

    for(int i = 0; i < qtd_atualizacoes; i++) {
        fscanf(stdin, "%d", &RRN);
        fscanf(stdin, "%s", campo);
        fscanf(stdin, "%s", valor);
        fgetc(stdin);

        rewind(file_bin);
        fseek(file_bin, TAM_CAB, SEEK_CUR);
        fseek(file_bin, RRN*TAM_REG, SEEK_CUR);

        reg = calloc(1, sizeof(Registro));
        aux = ler_reg(file_bin, reg);
        if(!aux)
            continue;

        if(!strcmp(campo, UF_DEST)) {
            strcpy(reg->UF_dest, valor);
        } else if(!strcmp(campo, UF_ORIG)) {
            strcpy(reg->UF_orig, valor);
        } else if(!strcmp(campo, DISTANCIA)) {
            reg->distancia = atoi(valor);
        } else if(!strcmp(campo, TEMPO)) {
            strcpy(reg->tempo, valor);
        } else if(!strcmp(campo, CIDADE_ORIG)) {
            strcpy(reg->cidade_orig, valor);
        } else if(!strcmp(campo, CIDADE_DEST)) {
            strcpy(reg->cidade_dest, valor);
        }
        //Salvar registro no RRN
    }

    fclose(file_bin);
}

void compact(char* nome_file_orig, char* nome_file_compac) {
}

char* get_data_sistema_formatada() {
    char* data = (char*) calloc(11, sizeof(char));
    time_t mytime;
    mytime = time(NULL);
    struct tm tm = *localtime(&mytime);
    tm.tm_mon += 1;
    tm.tm_year += 1900;
    snprintf(data, 11, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon, tm.tm_year);
    return data;
}

Registro_PTR ler_reg_csv(char c, FILE* file_csv) {
    Registro_PTR reg = (Registro_PTR)calloc(1, sizeof(Registro));
    reg->UF_orig[0] = c;
    fscanf(file_csv, "%c %*c %[^,] %*c %d %*c %[^,] %*c %[^,] %*c%[^\n]*c", &reg->UF_orig[1], reg->UF_dest, &reg->distancia, reg->cidade_orig, reg->cidade_dest, reg->tempo);
    fgetc(file_csv);

    return reg;
}

int salvar_registro(FILE* file_bin, Registro_PTR reg) {
    int tam_cidade_orig = strlen(reg->cidade_orig);
    int tam_cidade_dest = strlen(reg->cidade_dest);
    int tam_tempo = strlen(reg->tempo);
    int tam_lixo = TAM_REG - TAM_CAMPO_FIXO - (tam_cidade_dest+1) - (tam_cidade_orig+1) - (tam_tempo+1);
    if(tam_lixo < 0)
        return ERRO;

    fwrite(reg->UF_orig, 2*sizeof(char), 1, file_bin);
    fwrite(reg->UF_dest, 2*sizeof(char), 1, file_bin);
    fwrite(&reg->distancia, sizeof(int), 1, file_bin);
    fwrite(reg->cidade_orig, tam_cidade_orig * sizeof(char), 1, file_bin);
    fputc('|', file_bin);
    fwrite(reg->cidade_dest, tam_cidade_dest * sizeof(char), 1, file_bin);
    fputc('|', file_bin);
    fwrite(reg->tempo, tam_tempo * sizeof(char), 1, file_bin);
    fputc('|', file_bin);

    while(tam_lixo > 0) {
        fputc('#', file_bin);
        tam_lixo--;
    }
    return OK;
}

void exibir_bin(char* nome_bin, int RRN) {
    Registro_PTR reg;
    FILE* file_bin = fopen(nome_bin, "rb");
    if(!file_bin)
        printf("Falha no processamento do arquivo.");
    int aux, i = 0;

    fseek(file_bin, TAM_CAB, SEEK_CUR);
    if(RRN != TODOS) {
        reg = (Registro_PTR) calloc(1, sizeof(Registro));
        fseek(file_bin, RRN*TAM_REG, SEEK_CUR);
        aux = ler_reg(file_bin, reg);
        if(aux == OK)
            exibe_reg(RRN, reg);
        else if(aux == ERRO)
            printf("Registro inexistente.");
        free(reg);
    } else if(RRN == TODOS) {
        while(true) {
            reg = (Registro_PTR) calloc(1, sizeof(Registro));
            aux = ler_reg(file_bin, reg);
            if(aux == ERRO) {
                break;
            }
            exibe_reg(i++, reg);
            free(reg);
        }
    }
    fclose(file_bin);
}

int ler_reg(FILE* file_bin, Registro_PTR reg) {
    int tam_cidade_orig, tam_cidade_dest, tam_tempo;
    int aux;
    char* aux_campos = calloc(TAM_REG, sizeof(char));

    aux = fread(reg->UF_orig, 2*sizeof(char), 1, file_bin);
    aux = fread(reg->UF_dest, 2*sizeof(char), 1, file_bin);
    aux = fread(&reg->distancia, sizeof(int), 1, file_bin);
    aux = fread(aux_campos, (TAM_REG - TAM_CAMPO_FIXO)*sizeof(char), 1, file_bin);
    if(!aux)
        return ERRO;

    tam_cidade_orig = strcspn(&aux_campos[0], "|");
    tam_cidade_dest = strcspn(&aux_campos[tam_cidade_orig+1], "|");
    tam_tempo = strcspn (&aux_campos[tam_cidade_orig+1+tam_cidade_dest+1], "|");

    strncpy(reg->cidade_orig, &aux_campos[0], tam_cidade_orig);
    strncpy(reg->cidade_dest, &aux_campos[tam_cidade_orig+1], tam_cidade_dest);
    strncpy(reg->tempo, &aux_campos[tam_cidade_orig+1+tam_cidade_dest+1], tam_tempo);
    free(aux_campos);
    return OK;
}

void exibe_reg(int RRN, Registro_PTR reg) {
    printf("%d ", RRN);
    printf("%s ", reg->UF_orig);
    printf("%s ", reg->UF_dest);
    printf("%d ", reg->distancia);
    printf("%s ", reg->cidade_orig);
    printf("%s", reg->cidade_dest);
    if(strcmp(reg->tempo, ""))
        printf(" %s", reg->tempo);
    printf("\n");
}
