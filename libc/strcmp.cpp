int strcmp(char *str1, char *str2)
{
    while (*str1 && *str2 && (*str1++ == *str2++));

    if (*str1 == '\0' && *str2 == '\0')
        return 0;

    if (*str1 == '\0')
        return -1;
    else return 1;
}