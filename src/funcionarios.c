/*
 * Programa de cadastros de funcionários de uma empresa, como demonstração de
 * manipulação de arquivos sem ordenação com gerenciamento ciente de blocos.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

/* Registro utilizado para demonstrar o conceito */
struct funcionario {
    char codigo[6];
    char nome[40];
    char data_admissao[8];
    float salario;
};

/* Cada bloco do arquivo em disco (conjunto de registros) */
struct bloco_funcionarios {
    int contador;
    struct funcionario *vetor;
    char *lastro;
};

int TAMANHO_REGISTRO;
int REGISTROS_POR_BLOCO;
int TAMANHO_VETOR_BLOCO;
int TAMANHO_LASTRO;

void pesquisar(int arquivo, const char *codigo) {
    lseek(arquivo, 0, SEEK_SET);
}

void remover(int arquivo, const char *codigo) {
    lseek(arquivo, 0, SEEK_SET);
}

void inserir(int arquivo) {
    lseek(arquivo, 0, SEEK_SET);
}

int criar_arquivo(int quantos_blocos, struct bloco_funcionarios *b) {
    int arquivo = open("data/arquivo.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (arquivo < 0) {
        printf("Impossível abrir arquivo.\n");
        return 1;
    }

    int i;
    for (i = 0; i < quantos_blocos; i++) {
        if (write(arquivo, &b[i].contador, sizeof(int)) < 0) {
            printf("Erro na escrita.\n");
            return 1;
        }

        int j;
        for (j = 0; j < b[i].contador; j++) {
            if (write(arquivo, &b[i].vetor[j], TAMANHO_REGISTRO) < 0) {
                printf("Erro na escrita.\n");
                return 1;
            }
        }
        if (write(arquivo, &b[i].lastro, TAMANHO_LASTRO) < 0) {
            printf("Erro na escrita.\n");
            return 1;
        }
    }

    printf("\n");

    close(arquivo);

    return 0;
}

void inicializar_variaveis(const char * caminho_programa) {
    struct stat s;

    stat(caminho_programa, &s);

    int bloco_menos_contador = s.st_blksize - sizeof(int);

    TAMANHO_REGISTRO    = sizeof(struct funcionario);
    REGISTROS_POR_BLOCO = (int) bloco_menos_contador / TAMANHO_REGISTRO;
    TAMANHO_LASTRO      = bloco_menos_contador % TAMANHO_REGISTRO;
    TAMANHO_VETOR_BLOCO = REGISTROS_POR_BLOCO * TAMANHO_REGISTRO;

    printf("Tamanho do registro: %5d\n", TAMANHO_REGISTRO);
    printf("Registros por bloco: %5d\n", REGISTROS_POR_BLOCO);
    printf("Tamanho do lastro:   %5d\n", TAMANHO_LASTRO);
}


void novo_bloco(struct bloco_funcionarios *b, int c, struct funcionario *f, int *posicao) {
    printf("%d, %d\n", c, *posicao);
    b->contador = c;
    b->vetor    = (struct funcionario *)malloc(TAMANHO_VETOR_BLOCO);
    b->lastro   = (char *)malloc(TAMANHO_LASTRO);

    int i;
    for (i = 0; i < c; i++) {
        b->vetor[i] = f[*posicao];
        *posicao = *posicao + 1;
    }
}

struct bloco_funcionarios * separar_em_blocos(int quantos_registros, struct funcionario *f, int *qtd_blocos) {
    int quantos_blocos = (int) quantos_registros / REGISTROS_POR_BLOCO;
    int registros_no_ultimo_bloco = quantos_registros % REGISTROS_POR_BLOCO;

    if (registros_no_ultimo_bloco > 0) {
        quantos_blocos++;
    }
    else {
        registros_no_ultimo_bloco = REGISTROS_POR_BLOCO;
    }

    *qtd_blocos = quantos_blocos;

    struct bloco_funcionarios *blocos;
    blocos = (struct bloco_funcionarios *)malloc(quantos_blocos*sizeof(struct bloco_funcionarios));

    int posicao = 0;
    int i;
    for (i = 0; i < quantos_blocos-1; i++) {
        novo_bloco(&blocos[i], REGISTROS_POR_BLOCO, f, &posicao);
    }
    novo_bloco(&blocos[quantos_blocos-1], registros_no_ultimo_bloco, f, &posicao);

    return blocos;
}

int main(int argc, char *argv[]) {
    inicializar_variaveis(argv[0]);

    int quantos_registros = 2*REGISTROS_POR_BLOCO;
    struct funcionario *f;
    f = (struct funcionario *)malloc(sizeof(struct funcionario)*quantos_registros);

    int i;
    char data[9] = "20130117";
    for (i = 0; i < quantos_registros; i++) {
        sprintf(f[i].codigo, "abc%2d", i);
        sprintf(f[i].nome, "Funcionário %2d", i);
        int j;
        for (j = 0; j < 9; j++) {
            f[i].data_admissao[j] = data[j];
        }
        f[i].salario = 2000.00;
    }

    struct bloco_funcionarios *blocos;
    int quantos_blocos;
    blocos = separar_em_blocos(quantos_registros, f, &quantos_blocos);

    return criar_arquivo(quantos_blocos, blocos);
}
