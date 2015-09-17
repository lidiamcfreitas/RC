//
//  ECP.c
//  
//
//  Created by Lídia Maria Carvalho de Freitas on 17/09/15.
//
//

#include "User.h"

int main(){
    char buffer[128];
    gethostname(buffer, 128);
    printf("%s\n", buffer);
}
