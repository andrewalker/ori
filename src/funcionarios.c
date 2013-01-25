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
int TAMANHO_BLOCO_DISCO;

void listar_blocos(int quantos_blocos, struct bloco_funcionarios *b) {
    int i = 0;
    for (i = 0; i < quantos_blocos; i++) {
        printf("Bloco %d tem %d registros.\n", i, b[i].contador);
        int j;
        for (j = 0; j < b[i].contador; j++) {
            int k = 0;
            for (k = 0; k < 6; k++)
                printf("%c", b[i].vetor[j].codigo[k]);
            printf("\n");
        }
    }
}

void novo_bloco(struct bloco_funcionarios *b, int c, struct funcionario *f, int *posicao) {
    b->contador = c;
    b->vetor    = (struct funcionario *)malloc(TAMANHO_VETOR_BLOCO);
    b->lastro   = (char *)malloc(TAMANHO_LASTRO);

    int i;
    for (i = 0; i < c; i++) {
        b->vetor[i] = f[*posicao];
        *posicao = *posicao + 1;
    }
}

int encontrar_registro(int quantos_blocos, struct bloco_funcionarios *b, const char *codigo, int *bloco, int *registro) {
    int i;
    for (i = 0; i < quantos_blocos; i++) {
        int j;
        for (j = 0; j < b[i].contador; j++) {
            char igual = 1;
            int k;
            for (k = 0; igual && k < 6; k++) {
                if (b[i].vetor[j].codigo[k] != codigo[k])
                    igual = 0;
            }
            if (igual) {
                *bloco = i;
                *registro = j;
                return 1;
            }
        }
    }
    return 0;
}

int remover(int arquivo, int *quantos_blocos, struct bloco_funcionarios **b, const char *codigo) {
    int bloco, registro;

    if (!encontrar_registro(*quantos_blocos, *b, codigo, &bloco, &registro)) {
        printf("registro não encontrado\n");
        return 0;
    }

    (*b)[bloco].vetor[registro] = (*b)[*quantos_blocos - 1].vetor[(*b)[*quantos_blocos - 1].contador-1];

    (*b)[*quantos_blocos - 1].contador = (*b)[*quantos_blocos - 1].contador-1;

    lseek(arquivo, TAMANHO_BLOCO_DISCO*bloco, SEEK_SET);
    int retorno1 = escrever_bloco_em_arquivo(arquivo, (*b)[bloco]);

    lseek(arquivo, TAMANHO_BLOCO_DISCO*(*quantos_blocos-1), SEEK_SET);
    int retorno2 = escrever_bloco_em_arquivo(arquivo, (*b)[*quantos_blocos - 1]);

    int pos = lseek(arquivo, 0, SEEK_CUR);
    ftruncate(arquivo, pos);

    return retorno1 && retorno2;
}

int inserir(int arquivo, int *quantos_blocos, struct bloco_funcionarios **b, struct funcionario f) {
    if ((*b)[*quantos_blocos - 1].contador == REGISTROS_POR_BLOCO) {
        *quantos_blocos = *quantos_blocos+1;

        struct bloco_funcionarios *blocos;
        blocos = (struct bloco_funcionarios *)malloc((*quantos_blocos)*sizeof(struct bloco_funcionarios));

        int i = 0;
        for (i = 0; i < *quantos_blocos-1; i++) {
            blocos[i] = (*b)[i];
        }

        int posicao = 0;
        novo_bloco(&blocos[ *quantos_blocos-1 ], 1, &f, &posicao);

        *b = blocos;
    }
    else {
        int ultimo_bloco = *quantos_blocos-1;
        int ultima_posicao_do_bloco = (*b)[ultimo_bloco].contador;

        (*b)[ultimo_bloco].vetor[ ultima_posicao_do_bloco ] = f;
        (*b)[ultimo_bloco].contador = ultima_posicao_do_bloco+1;
    }

    lseek(arquivo, TAMANHO_BLOCO_DISCO*(*quantos_blocos-1), SEEK_SET);
    return escrever_bloco_em_arquivo(arquivo, (*b)[*quantos_blocos-1]);
}

int escrever_bloco_em_arquivo(int arquivo, struct bloco_funcionarios b) {
    if (write(arquivo, &b.contador, sizeof(int)) < 0) {
        printf("Erro na escrita.\n");
        return 0;
    }

    int j;
    for (j = 0; j < b.contador; j++) {
        if (write(arquivo, &b.vetor[j], TAMANHO_REGISTRO) < 0) {
            printf("Erro na escrita.\n");
            return 0;
        }
    }
    if (write(arquivo, &b.lastro, TAMANHO_LASTRO) < 0) {
        printf("Erro na escrita.\n");
        return 0;
    }

    return 1;
}

int criar_arquivo(int arquivo, int quantos_blocos, struct bloco_funcionarios *b) {
    int i;

    for (i = 0; i < quantos_blocos; i++) {
        if (!escrever_bloco_em_arquivo(arquivo, b[i])) {
            return 0;
        }
    }

    return 1;
}

void inicializar_variaveis(const char * caminho_programa) {
    struct stat s;

    stat(caminho_programa, &s);

    int bloco_menos_contador = s.st_blksize - sizeof(int);

    TAMANHO_REGISTRO    = sizeof(struct funcionario);
    REGISTROS_POR_BLOCO = (int) bloco_menos_contador / TAMANHO_REGISTRO;
    TAMANHO_LASTRO      = bloco_menos_contador % TAMANHO_REGISTRO;
    TAMANHO_VETOR_BLOCO = REGISTROS_POR_BLOCO * TAMANHO_REGISTRO;
    TAMANHO_BLOCO_DISCO = s.st_blksize;

    printf("Tamanho do registro: %5d\n", TAMANHO_REGISTRO);
    printf("Registros por bloco: %5d\n", REGISTROS_POR_BLOCO);
    printf("Tamanho do lastro:   %5d\n", TAMANHO_LASTRO);
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
        sprintf(f[i].codigo, "abc%d", i);
        sprintf(f[i].nome, "Funcionário %d", i);
        int j;
        for (j = 0; j < 9; j++) {
            f[i].data_admissao[j] = data[j];
        }
        f[i].salario = 2000.00;
    }

    struct bloco_funcionarios *blocos;
    int quantos_blocos;
    blocos = separar_em_blocos(quantos_registros, f, &quantos_blocos);

    int arquivo = open("data/arquivo.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (arquivo < 0) {
        printf("Impossível abrir arquivo.\n");
        return 0;
    }

    criar_arquivo(arquivo, quantos_blocos, blocos);

    struct funcionario f2;
    sprintf(f2.codigo, "abc%d", 200);
    sprintf(f2.nome, "Funcionário %d", 200);
    for (i = 0; i < 9; i++) {
        f2.data_admissao[i] = data[i];
    }
    f2.salario = 2000.00;

    inserir(arquivo, &quantos_blocos, &blocos, f2);
    inserir(arquivo, &quantos_blocos, &blocos, f2);

    if (remover(arquivo, &quantos_blocos, &blocos, "abc95")) {
        printf("Removeu\n");
    }

    listar_blocos(quantos_blocos, blocos);

    close(arquivo);
}
