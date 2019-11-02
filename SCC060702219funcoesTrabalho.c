
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>



/*
* Abaixo seguem funções que fazem a escrita do binário em "stdout" (tela) pra poder ser comparado no run.codes.
*
* Funciona assim: você faz tudo o que tiver que fazer na funcionalidade no arquivo em disco, assim como ensinado nas aulas da disciplina.
* Ao fim da funcionalidade, use a função "binarioNaTela" e a função já cuida de tudo para você. É só chamar a função.
*
* Note que ao usar a binarioNaTela1, o fclose no arquivo binário já deve ter sido feito anteriormente. Você passa o nome do arquivo binário ("arquivoTrabX.bin") pra ela e ela vai ler tudo e escrever na tela.
*
* Você pode colocar isso num módulo .h separado, ou incluir as funções no próprio código .c: como preferir.
* VOCÊ NÃO PRECISA ENTENDER ESSAS FUNÇÕES. É só usar elas da forma certa depois de acabar a funcionalidade.
*
* Tá tudo testado e funcionando, mas qualquer dúvida acerca dessas funções, falar com o monitor Matheus (mcarvalhor@usp.br).
*/



	// Se você for incluir no .h separado, tá abaixo:

	#ifndef H_ESCREVERNATELA_
	#define H_ESCREVERNATELA_
	#include <stdio.h>

	void binarioNaTela1(char *nomeArquivoBinario);
	void trim(char *str);
	void scan_quote_string(char *str);

	#endif

	// Acabou o código que vai no .h



// Abaixo vai em algum .c





