default:
	@mkdir -p {bin,data}
	gcc -o bin/funcionarios src/funcionarios.c

clean:
	rm -rf {bin,data}
