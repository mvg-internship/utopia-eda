
//     /*find init*/
//     for(auto i : maps) {
//         for(auto j : maps) {
//             int k = 0; 
//             if(i != j && i.definite == 0 && strlen(yytext) == strlen(i.name) && j.definite == 1) {
//                 for (int j = 0; j < strlen(yytext); j++) {
//                         if (i.name[j] == yytext[j])
//                             k += 1;
//                     }
//                     if (k == strlen(yytext)) {
//                         i.definite = 1; 
//                     }
//             }
//         }
//     }
// }