/*****************************************|
|**    TRABALHO ESTUTURA DE DADOS 3     **|
|** Itegrantes:                NUSP:    **|
|** Felipe Caldana Ricioli     10716654 **|
|** Helbert Moreira Pinto      10716504 **|
|** Higor Tessari              10345251 **|
|*****************************************/

/** Include's */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>


/** Define's de Tamanho */
#define TAM_REG 85
#define TAM_CAB 19
#define TAM_CAMPO_FIXO 8
#define TAM_COMANDO 100

/** Define's de Campos */
#define UF_ORIG "estadoOrigem"
#define UF_DEST "estadoDestino"
#define CIDADE_ORIG "cidadeOrigem"
#define CIDADE_DEST "cidadeDestino"
#define DISTANCIA "distancia"
#define TEMPO "tempoViagem"

/** Define's de Funcoes */
#define TRANSFER            1
#define READ_ALL            2
#define READ_FIL            3
#define READ_RRN            4
#define DELETE_STATIC_FIL   5
#define INCLUDE_STATIC      6
#define REALSE_RRN          7
#define COMPACT             8

/** Outros Define's */
#define OK 0
#define ERRO 1
#define STATUS_OK '1'
#define REMOVIDO '*'
#define TODOS -1
#define FILTRO -2
#define VAZIO ""


/** Struct de Cabecalho */
typedef struct {
    char status;
    int numeroVertices;
    int numeroArestas;
    char dataUltimaCompactacao[10];
} Cabecalho;
typedef Cabecalho* Cabecalho_PTR;

/** Struct de Registro */
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

/** Struct de Vertice [Auxliar] */
typedef struct {
    char nome_vertice[20];
    int qtd_rep;
} Vertice;
typedef Vertice* Vertice_PTR;
typedef Vertice** Vertice_PTR_PTR;

/** Struct de Lista [Vertice] */
typedef struct {
    Vertice_PTR_PTR lista_vertice;
    int numeroVertices;
    int numeroArestas;
} ListaArestaVertice;
typedef ListaArestaVertice* ListaArestaVertice_PTR;


/** Prototipos de funcoes */
void atualizar_cabecalho(FILE*, ListaArestaVertice_PTR, Cabecalho_PTR);
void inserir_aresta(ListaArestaVertice_PTR, char*);
int salvar_reg_bin(FILE*, Registro_PTR, bool);
void escrever_cabecalho(FILE*, Cabecalho_PTR);
void transfere_dados_csv_bin(char*, char*);
void atualizar_campo_registro(char*, int);
void exibir_bin(char*, int, char*, char*);
void get_data_sistema_formatada(char*);
void remove_reg_filtro(char*, int);
void exibe_reg(int, Registro_PTR);
int ler_reg(FILE*, Registro_PTR);
void scan_quote_string(char*);
void inserir_reg(char*, int);
bool arquivo_integro(FILE*);
void compact(char*, char*);
void binarioNaTela1(char*);

/** Funcao principal */
int main() {
    int funcao, param;
    char param_1[50], param_2[50], param_3[50];

    fscanf(stdin, "%d", &funcao);

    switch(funcao) {
    case TRANSFER:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%s", param_2);
        fgetc(stdin);

        transfere_dados_csv_bin(param_1, param_2);
        break;

    case READ_ALL:
        fscanf(stdin, "%s", param_1);
        fgetc(stdin);

        exibir_bin(param_1, TODOS, "", "");
        break;

    case READ_FIL:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%s", param_2);
        scan_quote_string(param_3);

        exibir_bin(param_1, FILTRO, param_2, param_3);
        break;

    case READ_RRN:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        exibir_bin(param_1, param, "", "");
        break;

    case DELETE_STATIC_FIL:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        remove_reg_filtro(param_1, param);
        break;

    case INCLUDE_STATIC:
        fscanf(stdin, "%s", param_1);
        fscanf(stdin, "%d", &param);
        fgetc(stdin);

        inserir_reg(param_1, param);
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
        printf("Erro no operador de funcao\n");
    }
    return 0;
}

/** FUNC[1] - Ler dados arquivo.CSV e salvar arquivo.BIN */
void transfere_dados_csv_bin(char* nome_csv, char* nome_bin) {
    FILE* file_csv = fopen(nome_csv, "r");   //Abre arquivo.CSV para leitura.
    FILE* file_bin = fopen(nome_bin, "wb+"); //Abre arquivo.BIN para escrita/leitura.
    if(!file_csv || !file_bin) {
        printf("Falha no carregamento do arquivo.");
        return;
    }

    ListaArestaVertice_PTR lista;
    Cabecalho_PTR cab;
    Registro_PTR reg;

    //Pular cabecalho arquivo.CSV
    fscanf(file_csv, "%*[^\n]");
    fgetc(file_csv);

    //Escrever cabecalho arquivo.BIN. Todos campos 0[NULL].
    cab = (Cabecalho_PTR) calloc(1, sizeof(Cabecalho));
    escrever_cabecalho(file_bin, cab);

    //Ler registro no arquivo.CSV e salvar arquivo.BIN. Um registro por vez.
    while(true) {
        reg = (Registro_PTR) calloc(1, sizeof(Registro));
        fscanf(file_csv, "%[^,] %*c %[^,] %*c %d %*c %[^,] %*c %[^,] %*c%[^\n]*c", reg->UF_orig, reg->UF_dest, &reg->distancia, reg->cidade_orig, reg->cidade_dest, reg->tempo);
        if(reg->UF_orig[0] < 'A' || reg->UF_orig[0] > 'Z') break; //Como estado origem nao pode ser nulo, se a primeira letra nao estiver no range e pq acabou o arquivo.
        fgetc(file_csv);
        salvar_reg_bin(file_bin, reg, true);
        free(reg);
    }

    //Contar Vertices e Arestas e reescrever cabecalho com dados computados.
    lista = (ListaArestaVertice_PTR) calloc(1, sizeof(ListaArestaVertice));
    strcpy(cab->dataUltimaCompactacao, "##########");
    atualizar_cabecalho(file_bin, lista, cab);
    free(lista);
    free(cab);

    fclose(file_csv);
    fclose(file_bin);
    binarioNaTela1(nome_bin);
}

/** FUNC[2] - Resgatar todos registros arquivo.BIN                          */
/** FUNC[3] - Resgatar registros arquivo.BIN com filtro de valor de campo   */
/** FUNC[4] - Resgatar registros arquivo.BIN com filtro por RRN             */
void exibir_bin(char* nome_bin, int RRN, char* campo, char* valor) {
    FILE* file_bin = fopen(nome_bin, "rb"); //Abre arquivo.BIN para leitura.

    //Verificar existencia e integridade do arquivo
    if(!file_bin || !arquivo_integro(file_bin)){
        printf("Falha no processamento do arquivo.");
        return;
    }

    Registro_PTR reg;
    int aux, i = 0;
    bool cond, achou = false;

    //Pulando restante do cabecalho arquivo.BIN
    fseek(file_bin, TAM_CAB-1, SEEK_CUR);

    if(RRN >= 0) {                                                      //BUSCA POR RRN
        reg = (Registro_PTR) calloc(1, sizeof(Registro));
        fseek(file_bin, RRN*TAM_REG, SEEK_CUR);              //Posicao do registro [O(1) - Constante]
        aux = ler_reg(file_bin, reg);
        if(aux == OK && reg->UF_orig[0] != REMOVIDO) {      //Verifica se leu e se registro nao foi removido
            exibe_reg(RRN, reg);
            achou = true;
        }
        free(reg);

    } else if(RRN == TODOS || RRN == FILTRO) {                          //BUSCA POR FILTRO OU TODOS
        while(true) {                                       //Ler 1 a 1 [O(n) - Linear]
            reg = (Registro_PTR) calloc(1, sizeof(Registro));
            aux = ler_reg(file_bin, reg);
            if(aux == ERRO){                                //Acabou arquivo
                free(reg);
                break;
            }

            if(RRN == FILTRO) {                                         //BUSCA POR FILTRO

                cond = (!strcmp(campo, UF_DEST) && !strcmp(reg->UF_dest, valor)) ||
                       (!strcmp(campo, UF_ORIG) && !strcmp(reg->UF_orig, valor)) ||
                       (!strcmp(campo, DISTANCIA) && strcmp(valor, VAZIO) && reg->distancia == atoi(valor)) ||
                       (!strcmp(campo, TEMPO) && !strcmp(reg->tempo, valor)) ||
                       (!strcmp(campo, CIDADE_ORIG) && !strcmp(reg->cidade_orig, valor)) ||
                       (!strcmp(campo, CIDADE_DEST) && !strcmp(reg->cidade_dest, valor));

                //Se a condicao for satisfeita e registro nao removido, exibir registro
                if(cond && reg->UF_orig[0] != REMOVIDO) {
                    exibe_reg(i, reg);
                    achou = true;
                }

            } else if(RRN == TODOS && reg->UF_orig[0] != REMOVIDO) {    //TODOS
                exibe_reg(i, reg);
                achou = true;
            }
            i++;
            free(reg);
        }
    }

    if(!achou)          //Se nao exibiu nenhum registro, exibir mensagem de registro inexistente.
        printf("Registro inexistente.");
    fclose(file_bin);
}

/** FUNC[5] - Remover registros arquivos.BIN com filtro de valor de campo */
void remove_reg_filtro(char* nome_file_bin, int qtd_it) {
    FILE* file_bin = fopen(nome_file_bin, "rb+");   //Abre arquivo.BIN para leitura/escrita.

    //Verificar existencia e integridade do arquivo
    if(!file_bin || !arquivo_integro(file_bin)) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    bool cond;
    int aux;
    char campo[50], valor[50];
    Cabecalho_PTR cab;
    Registro_PTR reg;
    ListaArestaVertice_PTR lista;

    //Escrever cabecalho arquivo.BIN
    rewind(file_bin);
    cab = (Cabecalho_PTR) calloc(1, sizeof(Cabecalho));
    fwrite(&cab->status, sizeof(char), 1, file_bin);
    fseek(file_bin, 2 * sizeof(int), SEEK_CUR);
    fread(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

    //Removendo registros
    for(int i = 0; i < qtd_it; i++) {
        fscanf(stdin, "%s", campo);     //Pegando filtro de remocao
        scan_quote_string(valor);

        rewind(file_bin);                   //Indo para o inicio
        fseek(file_bin, TAM_CAB, SEEK_CUR); //Pulando cabecalho.
        while(true) {
            reg = (Registro_PTR) calloc(1, sizeof(Registro));
            aux = ler_reg(file_bin, reg);
            if(aux == ERRO) { //Terminou arquivo
                free(reg);
                break;
            }else if(reg->UF_orig[0] == REMOVIDO) { //Pular registros ja removidos
                free(reg);
                continue;
            }

            cond = (!strcmp(campo, UF_DEST) && !strcmp(reg->UF_dest, valor)) ||
                   (!strcmp(campo, UF_ORIG) && !strcmp(reg->UF_orig, valor)) ||
                   (!strcmp(campo, DISTANCIA) && strcmp(valor, VAZIO) && reg->distancia == atoi(valor)) ||
                   (!strcmp(campo, TEMPO) && !strcmp(reg->tempo, valor)) ||
                   (!strcmp(campo, CIDADE_ORIG) && !strcmp(reg->cidade_orig, valor)) ||
                   (!strcmp(campo, CIDADE_DEST) && !strcmp(reg->cidade_dest, valor));
            //Se a condicao for satisfeita, remover registro
            if(cond) {
                fseek(file_bin, -(TAM_REG), SEEK_CUR);
                fputc(REMOVIDO, file_bin);
                fseek(file_bin, (TAM_REG-1), SEEK_CUR);
            }
            free(reg);
        }
    }

    //Contar Vertices e Arestas e reescrever cabecalho com dados computados.
    lista = (ListaArestaVertice_PTR) calloc(1, sizeof(ListaArestaVertice));
    atualizar_cabecalho(file_bin, lista, cab);
    free(lista);
    free(cab);

    fclose(file_bin);
    binarioNaTela1(nome_file_bin);
}

/** FUNC[6] - Inserir registro em arquivo.BIN nos locais onde existem registros removidos */
void inserir_reg(char* nome_file_bin, int qtd_inserir){
    FILE* file_bin = fopen(nome_file_bin, "rb+");   //Abre arquivo.BIN para leitura/escrita.

    //Verificar existencia e integridade do arquivo
    if(!file_bin || !arquivo_integro(file_bin)){
        printf("Falha no processamento do arquivo.");
        return;
    }

    Cabecalho_PTR cab;
    Registro_PTR reg;
    ListaArestaVertice_PTR lista;

    //Escrevendo cabecalho .BIN
    rewind(file_bin);
    cab = (Cabecalho_PTR) calloc(1, sizeof(Cabecalho));
    fwrite(&cab->status, sizeof(char), 1, file_bin);
    fseek(file_bin, 2*sizeof(int), SEEK_CUR);
    fread(cab->dataUltimaCompactacao, 10*sizeof(char), 1, file_bin);

    //Inserindo registros
    for(int i = 0; i < qtd_inserir; i++){
        reg = (Registro_PTR) calloc(1, sizeof(Registro));   //Pegando dados do registro a ser inserido
        fscanf(stdin, "%s", reg->UF_orig);
        fscanf(stdin, "%s", reg->UF_dest);
        fscanf(stdin, "%d", &reg->distancia);
        scan_quote_string(reg->cidade_orig);
        scan_quote_string(reg->cidade_dest);
        scan_quote_string(reg->tempo);

        //Insercao estatica => no fim do arquivo
        fseek(file_bin, 0, SEEK_END);       //Pulando para o fim do arquivo

        //Salvando novo registro
        salvar_reg_bin(file_bin, reg, true); //Salvando registro
        free(reg);
    }

    //Contar Vertices e Arestas e reescrever cabecalho com dados computados.
    lista = (ListaArestaVertice_PTR) calloc(1, sizeof(ListaArestaVertice));
    atualizar_cabecalho(file_bin, lista, cab);
    free(lista);
    free(cab);

    fclose(file_bin);
    binarioNaTela1(nome_file_bin);
}

/** FUNC[7] - Atualizar registro em arquivo.BIN com filtro de valor de campo */
void atualizar_campo_registro(char* nome_file, int qtd_atualizacoes) {
    FILE* file_bin = fopen(nome_file, "rb+");   //Abre arquivo.BIN para leitura/escrita.

    //Verificar existencia e integridade do arquivo
    if(!file_bin || !arquivo_integro(file_bin)) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    int RRN, aux;
    char campo[50], valor[50];
    ListaArestaVertice_PTR lista;
    Registro_PTR reg;
    Cabecalho_PTR cab;

    //Escrevendo cabecalho .BIN
    rewind(file_bin);
    cab = (Cabecalho_PTR) calloc(1, sizeof(Cabecalho));
    fwrite(&cab->status, sizeof(char), 1, file_bin);
    fseek(file_bin, 2 * sizeof(int), SEEK_CUR);
    fread(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

    //Realizando alteracoes
    for(int i = 0; i < qtd_atualizacoes; i++) {
        fscanf(stdin, "%d", &RRN);      //Pegando RRN e dados que serao alterados
        fscanf(stdin, "%s", campo);
        scan_quote_string(valor);

        //Posicionando o ponteiro na posicao onde sera alterado o registro
        rewind(file_bin);
        fseek(file_bin, TAM_CAB, SEEK_CUR);
        fseek(file_bin, RRN*TAM_REG, SEEK_CUR);

        reg = (Registro_PTR) calloc(1, sizeof(Registro));
        aux = ler_reg(file_bin, reg);       //Le o registro a ser alterado
        if(aux == ERRO || reg->UF_orig[0] == REMOVIDO) {
            free(reg);
            continue;
        }

        //Alterar o campo
        if(!strcmp(campo, UF_DEST)) {
            strcpy(reg->UF_dest, valor);
        } else if(!strcmp(campo, UF_ORIG)) {
            strcpy(reg->UF_orig, valor);
        } else if(!strcmp(campo, DISTANCIA) && strcmp(valor, VAZIO)) {
            reg->distancia = atoi(valor);
        } else if(!strcmp(campo, TEMPO)) {
            strcpy(reg->tempo, valor);
        } else if(!strcmp(campo, CIDADE_ORIG)) {
            strcpy(reg->cidade_orig, valor);
        } else if(!strcmp(campo, CIDADE_DEST)) {
            strcpy(reg->cidade_dest, valor);
        }

        //Reescreve registro
        fseek(file_bin, -(TAM_REG), SEEK_CUR);
        salvar_reg_bin(file_bin, reg, false);
        free(reg);
    }

    //Contar Vertices e Arestas e reescrever cabecalho com dados computados.
    lista = (ListaArestaVertice_PTR) calloc(1, sizeof(ListaArestaVertice));
    atualizar_cabecalho(file_bin, lista, cab);
    free(lista);
    free(cab);

    fclose(file_bin);
    binarioNaTela1(nome_file);
}

/** FUNC[8] - Compactar arquivo.BIN, removendo fisicamente os registros removidos */
void compact(char* nome_file_orig, char* nome_file_compac) {
    FILE* file_bin = fopen(nome_file_orig, "rb");           //Abre arquivo.BIN para leitura.
    FILE* file_bin_compac = fopen(nome_file_compac, "wb");  //Abre arquivo_compac.BIN para escrita.

    //Verificar existencia e integridade do arquivo
    if(!file_bin || !file_bin_compac || !arquivo_integro(file_bin)) {
        printf("Falha no carregamento do arquivo.");
        return;
    }

    char* data;
    int aux;
    Registro_PTR reg;
    Cabecalho_PTR cab;

    //Lendo cabecalho arquivo.BIN
    cab = (Cabecalho_PTR) calloc(1, sizeof(Cabecalho));
    fread(&cab->numeroVertices, sizeof(int), 1, file_bin);
    fread(&cab->numeroArestas, sizeof(int), 1, file_bin);
    fread(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

    //Escrevendo cabecalho arquivo_compac.BIN
    escrever_cabecalho(file_bin_compac, cab);

    while(true) {               //Passando por todos os registros de arquivo.BIN
        reg = (Registro_PTR) calloc(1, sizeof(Registro));
        aux = ler_reg(file_bin, reg);
        if(aux == ERRO) {       //Terminou arquivo.BIN
            free(reg);
            break;
        }else if(reg->UF_orig[0] == REMOVIDO) {   //Pulando removidos[Compactando]
            free(reg);
            continue;
        }
        salvar_reg_bin(file_bin_compac, reg, true); //Salvando registro em arquivo_compac.BIN
        free(reg);
    }

    //Reescrevendo cabecalho arquivo_compac.BIN
    rewind(file_bin_compac);
    cab->status = STATUS_OK;
    data = (char*) calloc(11, sizeof(char));
    get_data_sistema_formatada(data);
    strcpy(cab->dataUltimaCompactacao, data);
    free(data);
    escrever_cabecalho(file_bin_compac, cab);
    free(cab);

    fclose(file_bin);
    fclose(file_bin_compac);
    binarioNaTela1(nome_file_compac);
}

/** Funcao pega a data do sistema */
void get_data_sistema_formatada(char* data) {
    time_t mytime;
    mytime = time(NULL);
    struct tm tm = *localtime(&mytime);
    tm.tm_mon += 1;
    tm.tm_year += 1900;
//   snprintf(data, 11, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon, tm.tm_year);   //Data nao fixada
    snprintf(data, 11, "01/%02d/%04d", tm.tm_mon, tm.tm_year);                  //Data fixada [RUNCODES]
}

/** Salva um registro em modo binario no arquivo */
int salvar_reg_bin(FILE* file_bin, Registro_PTR reg, bool alt_lixo) {
    int tam_cidade_orig = strlen(reg->cidade_orig);
    int tam_cidade_dest = strlen(reg->cidade_dest);
    int tam_tempo = strlen(reg->tempo);
    int tam_lixo = TAM_REG - TAM_CAMPO_FIXO - (tam_cidade_dest+1) - (tam_cidade_orig+1) - (tam_tempo+1); //Todos tem +1 pelo espaco do '|' [PIPE]
    if(tam_lixo < 0)       //Necessario fazer um truncamento [Nao necessario no trab]
        return ERRO;

    fwrite(reg->UF_orig, 2*sizeof(char), 1, file_bin);
    fwrite(reg->UF_dest, 2*sizeof(char), 1, file_bin);
    fwrite(&reg->distancia, sizeof(int), 1, file_bin);
    fwrite(reg->cidade_orig, tam_cidade_orig * sizeof(char), 1, file_bin); fputc('|', file_bin);
    fwrite(reg->cidade_dest, tam_cidade_dest * sizeof(char), 1, file_bin); fputc('|', file_bin);
    fwrite(reg->tempo, tam_tempo * sizeof(char), 1, file_bin);             fputc('|', file_bin);
    if(alt_lixo)    //true -> Se for para alterar/inserir lixo
        for(int i = 0; i < tam_lixo; i++)
            fputc('#', file_bin);
    return OK;
}

/** Le um registro em modo binario do arquivo */
int ler_reg(FILE* file_bin, Registro_PTR reg) {
    int tam_cidade_orig, tam_cidade_dest, tam_tempo;
    int aux;
    char* aux_campos;

    //CAMPOS TAMANHO FIXO
    aux = fread(reg->UF_orig, 2*sizeof(char), 1, file_bin);
    aux = fread(reg->UF_dest, 2*sizeof(char), 1, file_bin);
    aux = fread(&reg->distancia, sizeof(int), 1, file_bin);
    if(!aux)
        return ERRO;

    //Le todos os campos variaveis com o lixo, depois faz como um split quebrando os dados pelo separador '|' [PIPE]
    aux_campos = (char*) calloc(TAM_REG, sizeof(char));
    aux = fread(aux_campos, (TAM_REG - TAM_CAMPO_FIXO) * sizeof(char), 1, file_bin);

    //Cidade Origem
    tam_cidade_orig = strcspn(&aux_campos[0], "|");
    strncpy(reg->cidade_orig, &aux_campos[0], tam_cidade_orig);

    //Cidade Destino
    tam_cidade_dest = strcspn(&aux_campos[tam_cidade_orig+1], "|");
    strncpy(reg->cidade_dest, &aux_campos[tam_cidade_orig+1], tam_cidade_dest);

    //Tempo Viagem
    tam_tempo = strcspn(&aux_campos[tam_cidade_orig+tam_cidade_dest+2], "|");
    strncpy(reg->tempo, &aux_campos[tam_cidade_orig+tam_cidade_dest+2], tam_tempo);

    free(aux_campos);
    return OK;
}

/** Exibe um registro em tela, de acordo com as especificacoes do trabalho */
void exibe_reg(int RRN, Registro_PTR reg) {
    printf("%d %s %s %d %s %s", RRN, reg->UF_orig, reg->UF_dest, reg->distancia, reg->cidade_orig, reg->cidade_dest);
    if(strcmp(reg->tempo, ""))
        printf(" %s", reg->tempo);
    printf("\n");
}

/** Abre o arquivo do parametro.BIN
 * calcula o numero de vertices/arestas
 * reescreve os dados em vertives.BIN
 * Atualiza cabecalho
*/
void atualizar_cabecalho(FILE* file_bin, ListaArestaVertice_PTR lista, Cabecalho_PTR cab) {
    FILE* file_vert = fopen("vertices.bin", "wb");      //Abre vertices.BIN para reescrita
    if(!file_vert) {
        printf("Falha no processamento do arquivo de vertice.");
        return;
    }

    int tam_cidade_orig, tam_cidade_dest;
    char* campo_var;
    char* nome_cidade;
    char c;

    rewind(file_bin);
    fseek(file_bin, TAM_CAB, SEEK_CUR);     //Pula cabecalho .BIN

    while(true) {                           //Percorre todos registros do arquivo.BIN
        c = fgetc(file_bin);
        if(c == REMOVIDO){            //Verifica se registro corrente foi removido
            fseek(file_bin, TAM_REG - 1, SEEK_CUR);
            continue;
        }else if(c == EOF){   //Fim do arquivo
            break;
        }
        fseek(file_bin, TAM_CAMPO_FIXO - 1, SEEK_CUR);      //Pula os campos fixos - 1, pois ja esta lendo um caracter pra ver se foi removido
        campo_var = (char*) calloc(TAM_REG, sizeof(char));
        fread(campo_var, (TAM_REG - TAM_CAMPO_FIXO)*sizeof(char), 1, file_bin); //Le todo conteudo de campo variavel

        lista->numeroArestas += 1;

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

    //Salvar no arquivo os dados contidos na lista de vertices ordenada
    for(int i  = 0; i < lista->numeroVertices; i++) {
        fwrite(lista->lista_vertice[i]->nome_vertice, 20 * sizeof(char), 1, file_vert);
        fwrite(&lista->lista_vertice[i]->qtd_rep, sizeof(int), 1, file_vert);
    }
    fclose(file_vert);

    //Atuzalizando cabecalho
    cab->status = STATUS_OK;
    cab->numeroVertices = lista->numeroVertices;
    cab->numeroArestas = lista->numeroArestas;
    escrever_cabecalho(file_bin, cab);
}

/** Inserir/incrementar ao vetor de modo ordenado a Cidade do parametro */
void inserir_aresta(ListaArestaVertice_PTR lista, char* cidade) {
    Vertice_PTR novo;
    bool ja_existe_na_lista = false;
    int tam = lista->numeroVertices;
    int aux_cmp;
    int inf = 0, meio, sup = tam-1;
    int ind = 0;

    //Adaptacao ao metodo de busca binaria
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

    //Inserindo no vetor na posicao ordenada
    if(!ja_existe_na_lista) {
        novo = (Vertice_PTR) malloc(sizeof(Vertice));
        strcpy(novo->nome_vertice, cidade);
        novo->qtd_rep = 1;
        lista->lista_vertice = (Vertice_PTR_PTR) realloc(lista->lista_vertice, (tam+1) * sizeof(Vertice_PTR));
        lista->numeroVertices++;
        for(int i = tam; i > ind; i--)      //Shiftando todos elementos das posicoes posteriores ao elemento
            lista->lista_vertice[i] = lista->lista_vertice[i-1];
        lista->lista_vertice[ind] = novo;   //Inserindo o novo elemento no vetor
    }
}

/** Escreve o cabebalho */
void escrever_cabecalho(FILE* file_bin, Cabecalho_PTR cab) {
    rewind(file_bin);
    fwrite(&cab->status, sizeof(char), 1, file_bin);
    fwrite(&cab->numeroVertices, sizeof(int), 1, file_bin);
    fwrite(&cab->numeroArestas, sizeof(int), 1, file_bin);
    fwrite(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);
}

/** Verifica se arquivo esta integro para uso */
bool arquivo_integro(FILE* file_bin){
    rewind(file_bin);
    char status;
    fread(&status, sizeof(char), 1, file_bin);
    return (status == STATUS_OK);
}

/** Funcao que formata os dados de entrada para uso no arquivo.BIN */
void scan_quote_string(char *str) {
    char R;
    while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...
    if(R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar(); // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if(R == '\"') {
        if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar(); // ignorar aspas fechando
    } else if(R != EOF) { // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

/** Funcao que calcula o valor a ser exibido em tela de acordo com a posicao dos bytes no arquivo */
void binarioNaTela1(char *nomeArquivoBinario) {
    unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela1): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}
