#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define TAM_REG 85
#define TAM_CAMPO_FIXO 8
#define OK 0
#define ERRO 1

typedef struct{
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
	int tam_lista;
} ListaVertice;
typedef ListaVertice* ListaVertice_PTR;


/* Prototipos de funcoes */
void transfere_dados_csv_bin();
Registro_PTR ler_registro(char, FILE*);
int salvar_registro(FILE*, Registro_PTR);
void limpar_memoria(Registro_PTR);
void computar_vertice(ListaVertice_PTR, char*);
char* get_data_sistema_formatada();

int main(){

	transfere_dados_csv_bin();
	return 0;
}

char* get_data_sistema_formatada(){
    char* data = (char*) calloc(11, sizeof(char));
    time_t mytime;
    mytime = time(NULL);
    struct tm tm = *localtime(&mytime);
    tm.tm_mon += 1;
    tm.tm_year += 1900;
    snprintf(data, 11, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon, tm.tm_year);
    return data;
}

void transfere_dados_csv_bin() {
	FILE* file_csv = fopen("conjuntoDados.csv", "r");
	FILE* file_bin = fopen("conjuntoDados.bin", "wb");
	FILE* file_vert = fopen("vertices.bin", "wb");

	ListaVertice_PTR lista = calloc(1, sizeof(ListaVertice));
	Cabecalho_PTR cab = calloc(1, sizeof(Cabecalho));
    Registro_PTR reg;

	fscanf(file_csv, "%*[^\n]");
	fgetc(file_csv);

	//Escrever cabecalho
	fwrite(&cab->status, sizeof(char), 1, file_bin);
	fwrite(&cab->numeroVertices, sizeof(int), 1, file_bin);
	fwrite(&cab->numeroArestas , sizeof(int), 1, file_bin);
	fwrite(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

	char c;
	while((c = fgetc(file_csv)) != EOF) {
        reg = ler_registro(c, file_csv);   // LE registro
        computar_vertice(lista, reg->cidade_orig);
        computar_vertice(lista, reg->cidade_dest);
        salvar_registro(file_bin, reg); // SALVA registro
        limpar_memoria(reg);
        cab->numeroArestas++;
	}
	cab->status = '1';
	cab->numeroVertices = lista->tam_lista;
	char* data = get_data_sistema_formatada();
	strcpy(cab->dataUltimaCompactacao, data);
	free(data);

	//Reescrever cabecalho
	rewind (file_bin);
	fwrite(&cab->status, sizeof(char), 1, file_bin);
	fwrite(&cab->numeroVertices, sizeof(int), 1, file_bin);
	fwrite(&cab->numeroArestas , sizeof(int), 1, file_bin);
	fwrite(cab->dataUltimaCompactacao, 10 * sizeof(char), 1, file_bin);

	for(int i  = 0; i < lista->tam_lista; i++){
        fwrite(lista->lista_vertice[i]->nome_vertice, 20 * sizeof(char), 1, file_vert);
        fwrite(&lista->lista_vertice[i]->qtd_rep, sizeof(int), 1, file_vert);
	}

	fclose(file_csv);
	fclose(file_bin);
	fclose(file_vert);
}

Registro_PTR ler_registro(char c, FILE* file_csv){
    Registro_PTR reg = (Registro_PTR)calloc(1, sizeof(Registro));
    reg->UF_orig[0] = c;
    fscanf(file_csv, "%c %*c %[^,] %*c %d %*c %[^,] %*c %[^,] %*c%[^\n]*c", &reg->UF_orig[1], reg->UF_dest, &reg->distancia, reg->cidade_orig, reg->cidade_dest, reg->tempo);
    fgetc(file_csv);

    return reg;
}

int salvar_registro(FILE* file_bin, Registro_PTR reg){
    int tam_cidade_orig = strlen(reg->cidade_orig);
    int tam_cidade_dest = strlen(reg->cidade_dest);
    int tam_tempo = strlen(reg->tempo);
    int tam_lixo = TAM_REG - TAM_CAMPO_FIXO - (tam_cidade_dest+1) - (tam_cidade_orig+1) - (tam_tempo+1);
    if(tam_lixo < 0) return ERRO;

    fwrite(reg->UF_orig, 2*sizeof(char), 1, file_bin);
    fwrite(reg->UF_dest, 2*sizeof(char), 1, file_bin);
    fwrite(&reg->distancia, sizeof(int), 1, file_bin);
    fwrite(reg->cidade_orig, tam_cidade_orig * sizeof(char), 1, file_bin); fputc('|', file_bin);
    fwrite(reg->cidade_dest, tam_cidade_dest * sizeof(char), 1, file_bin); fputc('|', file_bin);
    fwrite(reg->tempo, tam_tempo * sizeof(char), 1, file_bin); fputc('|', file_bin);

    while(tam_lixo > 0){
        fputc('#', file_bin);
        tam_lixo--;
    }
    return OK;
}

void limpar_memoria(Registro_PTR reg){
    free(reg->cidade_dest);
    free(reg->cidade_orig);
    free(reg->tempo);
    free(reg);
}

void computar_vertice(ListaVertice_PTR lista, char* cidade){
    int tam = lista->tam_lista;
    int aux_cmp;
    int inf = 0;
    int sup = tam-1;
    int meio;
    int ind = 0;
    bool ja_existe_na_lista = false;
    Vertice_PTR novo;

    while (inf <= sup){
        meio = (int)(inf + sup)/2;
        aux_cmp = strcmp(cidade, lista->lista_vertice[meio]->nome_vertice);

        if(aux_cmp < 0){
            sup = meio - 1;
            ind = meio;
        } else if(aux_cmp > 0){
            inf = meio+1;
            ind = inf;
        } else{
            lista->lista_vertice[meio]->qtd_rep++;
            ja_existe_na_lista = true;
            break;
        }
    }

    if(!ja_existe_na_lista){
        novo = (Vertice_PTR) malloc(sizeof(Vertice));
        strcpy(novo->nome_vertice, cidade);
        novo->qtd_rep = 1;
        lista->lista_vertice = (Vertice_PTR_PTR) realloc(lista->lista_vertice, (tam+1) * sizeof(Vertice_PTR));
        lista->tam_lista++;
        for(int i = tam; i > ind; i--)
            lista->lista_vertice[i] = lista->lista_vertice[i-1];
        lista->lista_vertice[ind] = novo;
    }
}






















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

void trim(char *str) {
	size_t len;
	char *p;

	for(len = strlen(str); len > 0 && isspace(str[len - 1]); len--); // remove espaçamentos do fim
	str[len] = '\0';
	for(p = str; *p != '\0' && isspace(*p); p++); // remove espaçamentos do começo
	len = strlen(p);
	memmove(str, p, sizeof(char) * (len + 1));
}

void scan_quote_string(char *str) {
	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}
