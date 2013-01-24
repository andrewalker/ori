/*
 * Programa de cadastros de funcionários de uma empresa, como demonstração de
 * manipulação de arquivos sem ordenação com gerenciamento ciente de blocos.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>

/* Registro utilizado para demonstrar o conceito */
struct funcionario {
    char codigo[6];
    char nome[40];
    char data_admissao[8];
    float salario;
};

struct bloco_funcionarios {
    // TODO: int contador
    struct funcionario *vetor;
    char *lastro;
};

int main(int argc, char *argv[]) {
    struct stat fi;
    stat(argv[0], &fi);
    printf("Block size: %d\n", fi.st_blksize);
    printf("Struct size: %d\n", sizeof(struct funcionario));

    int tamanho = sizeof(struct funcionario);
    int funcionarios_por_bloco = (int)fi.st_blksize / tamanho;
    int lastro                 = fi.st_blksize % tamanho;

    printf("%d | %d\n", funcionarios_por_bloco, lastro);

    struct bloco_funcionarios b1;
    b1.vetor = (struct funcionario *)malloc(funcionarios_por_bloco * tamanho);
    b1.lastro = (char *)malloc(lastro);

    int i;
    char data[9] = "20130117";
    for (i = 0; i < funcionarios_por_bloco; i++) {
        sprintf(b1.vetor[i].codigo, "abc%2d", i);
        sprintf(b1.vetor[i].nome, "Funcionário %2d", i);
        int j;
        for (j = 0; j < 9; j++) {
            b1.vetor[i].data_admissao[j] = data[j];
        }
        b1.vetor[i].salario = 2000.00;
    }

    int a = open("data/arquivo.txt", O_WRONLY | O_CREAT);

    for (i = 0; i < funcionarios_por_bloco; i++) {
        if (write(a, &b1.vetor[i], tamanho) < 0) {
            printf("erro na escrita\n");
            return 1;
        }
        else {
            printf("%d ", i);
        }
    }
    if (write(a, &b1.lastro, lastro) < 0) {
        printf("erro na escrita\n");
        return 1;
    }

    close(a);

    return 0;
}
