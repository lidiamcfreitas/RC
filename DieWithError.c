//
//  DieWithError.c
//  
//
//  Created by Lídia Maria Carvalho de Freitas on 09/09/15.
//
//

#include <stdio.h>
#include <stdlib.h>

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}