#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define TAM_REG 85
#define TAM_CAMPO_FIXO 8
#define OK 0
#define ERRO 1
typedef struct{
	char UF_orig[3];
	char UF_dest[3];
	int distancia;
	char* cidade_orig;
	char* cidade_dest;
	char* tempo;
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

void transfere_dados_csv_bin();
Registro_PTR ler_registro(FILE*);
int salvar_registro(FILE*, Registro_PTR);
void limpar_memoria(Registro_PTR);

int main() {
	transfere_dados_csv_bin();
	return 0;
}

void transfere_dados_csv_bin() {
	FILE* file_csv = fopen("conjuntoDados.csv", "r");
	FILE* file_bin = fopen("conjuntoDados.bin", "wb");
	FILE* file_vert = fopen("vertices.bin", "wb");

    Registro_PTR reg;
    char aux[100];
	fgets(aux, 100, file_csv);  //Ler cabecalho

	while (!feof(file_csv)) {
        reg = ler_registro(file_csv);   // LE registro
//        computar_vertice(reg);
        salvar_registro(file_bin, reg); // SALVA registro
        limpar_memoria(reg);
	}


	fclose(file_csv);
	fclose(file_bin);
	fclose(file_vert);
}


Registro_PTR ler_registro(FILE* file_csv){
    Registro_PTR reg = (Registro_PTR)malloc(sizeof(Registro));
    int i;
    char c;

    fread(reg->UF_orig, 2*sizeof(char), 1, file_csv);
    reg->UF_orig[2] = '\0';
    fgetc(file_csv);

    fread(reg->UF_dest, 2*sizeof(char), 1, file_csv);
    reg->UF_dest[2] = '\0';
    fgetc(file_csv);

    fscanf(file_csv, "%d", &reg->distancia);
    fgetc(file_csv);

    i = 0;
    reg->cidade_orig = NULL;
    do{
        c = fgetc(file_csv);
        reg->cidade_orig = realloc(reg->cidade_orig, (++i) * sizeof(char));
        if(c == ','){
            reg->cidade_orig = realloc(reg->cidade_orig, (++i) * sizeof(char));
            reg->cidade_orig[i - 2] = '|';
            reg->cidade_orig[i - 1] = '\0';
            continue;
        }
        reg->cidade_orig[i - 1] = c;
    } while (c != ',');

    i = 0;
    reg->cidade_dest = NULL;
    do{
        c = fgetc(file_csv);
        reg->cidade_dest = realloc(reg->cidade_dest, (++i) * sizeof(char));
        if(c == ','){
            reg->cidade_dest = realloc(reg->cidade_dest, (++i) * sizeof(char));
            reg->cidade_dest[i - 2] = '|';
            reg->cidade_dest[i - 1] = '\0';
            continue;
        }
        reg->cidade_dest[i - 1] = c;
    } while (c != ',');

    i = 0;
    reg->tempo = NULL;
    do{
        c = fgetc(file_csv);
        reg->tempo = realloc(reg->tempo, (++i) * sizeof(char));
        if(c == '\n'){
            reg->tempo = realloc(reg->tempo, (++i) * sizeof(char));
            reg->tempo[i - 2] = '|';
            reg->tempo[i - 1] = '\0';
            continue;
        }
        reg->tempo[i - 1] = c;
    } while (c != '\n');

    return reg;
}

int salvar_registro(FILE* file_bin, Registro_PTR reg){
    char lixo = '#';
    int tam_cidade_orig = strlen(reg->cidade_orig);
    int tam_cidade_dest = strlen(reg->cidade_dest);
    int tam_tempo = strlen(reg->tempo);
    int tam_lixo = TAM_REG - TAM_CAMPO_FIXO - tam_cidade_dest - tam_cidade_orig - tam_tempo;
    if(tam_lixo < 0) return ERRO;

    fwrite(reg->UF_orig, 2*sizeof(char), 1, file_bin);
    fwrite(reg->UF_dest, 2*sizeof(char), 1, file_bin);
    fwrite(&reg->distancia, sizeof(int), 1, file_bin);
    fwrite(reg->cidade_orig, tam_cidade_orig * sizeof(char), 1, file_bin);
    fwrite(reg->cidade_dest, tam_cidade_dest * sizeof(char), 1, file_bin);
    fwrite(reg->tempo, tam_tempo * sizeof(char), 1, file_bin);
    while(tam_lixo > 0){
        fwrite(&lixo, sizeof(char), 1, file_bin);
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
