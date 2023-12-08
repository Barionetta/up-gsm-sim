// Biblioteki
#include <cstdio>
#include <cstdlib>
#include <winscard.h>

// Stale
#define MAX_READER_NAME_SIZE 40 // Dlugosc nazwy czytnika
#define MAX_ATR_SIZE 33         // ATR jest odpowiedzia na zerowanie karty. Rzadko wykorzystuje sie 33 bajty

// Program
int main(int argc, char **argv)
{
    // Zmienne
    SCARDCONTEXT hContext;                          // okresla zarzadzanie kontekstem zasobow
    SCARDHANDLE hCard;                              // zmienna obslugujaca polaczenie z karta
    SCARD_READERSTATE_A rgReaderStates[1];
    DWORD dwReaderLen, dwState, dwProt, dwAtrLen;   // dwReaderLen - dlugosc nazwy czytnika
                                                    // dwState - stan karty w czytniku
                                                    // dwProt - obecny protokol
                                                    // dwAtrLen - dlugosc bufora pbAtr
    DWORD dwPref, dwReaders, dwRespLen;             // dwPref - preferowany protokol
                                                    // dwReaders - bufory z nazwami czytnikow
                                                    // dwResplen - dlugosc odpowiedzi
    LPWSTR pcReaders;                               // lista nazw, pod ktorymi znany jest czytnik
    LPWSTR mszReaders;                              // lista czytnikow
    BYTE pbAtr[MAX_ATR_SIZE];                       // wskaznik na 32-bajtowy bufor ze stringiem ATR
    // ODPOWIEDZI
    BYTE pbResp1[30];
    BYTE pbResp2[30];
    BYTE pbResp3[30];
    BYTE pbResp4[30];
    BYTE pbResp5[200];
    LPCWSTR mszGroups;                               // lista grup czytnikow
    LONG rv;
    int i, p, iReader;                               // i - iterator
                                                     // p - ilosc czytnikow
                                                     // iReader - obecny czytnik
    int iReaders[16];                                // lista czytnikow zapisana w tablicy


    // Wyswietl czytniki
    printf("SCardEstablishContext : ");
    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &hContext);

    // Sprawdzansko
    if (rv != SCARD_S_SUCCESS)
    {
        printf("failed\n");
        return -1;
    }
    else printf("success\n");


    mszGroups = nullptr;                                // jezeli ta zmienna jest nullem, ma wyswietlic wszystkie czytniki
    printf("SCardListReaders : ");
    rv = SCardListReaders(hContext, mszGroups, nullptr, &dwReaders);

    // Sprawdzansko
    if (rv != SCARD_S_SUCCESS)
    {
        SCardReleaseContext(hContext);
        printf("failed\n");
        return -1;
    }
    else printf("success\n");

    mszReaders = (LPWSTR)malloc(sizeof(char) * dwReaders);

    printf("SCardListReaders : ");
    rv = SCardListReaders(hContext, mszGroups, mszReaders, &dwReaders);

    // Sprawdzansko
    if (rv != SCARD_S_SUCCESS)
    {
        SCardReleaseContext(hContext);
        free(mszReaders);
        printf("failed\n");
        return -1;
    }
    else printf("success\n");

    p = 0;
    for (i = 0; i < dwReaders - 1; ++i)
    {
        iReaders[++p] = i;
        printf("Reader %02d: %ls\n", p, &mszReaders[i]);
        while (mszReaders[++i] != '\0');
    }



    // Wybierz czytnik
    do
    {
        printf("Select reader : ");
        scanf_s("%d", &iReader);
    } while (iReader > p || iReader <= 0);

    printf("SCardConnect : ");
    SCardConnect(hContext, &mszReaders[iReaders[iReader]],
                 SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                 &hCard, &dwPref);

    // Sprawdzansko
    if (rv != SCARD_S_SUCCESS)
    {
        SCardReleaseContext(hContext);
        free(mszReaders);
        printf("failed\n");
        return -1;
    }
    else printf("success\n");



    // Wyswietl status karty
    printf("SCardStatus : ");
    dwReaderLen = MAX_READER_NAME_SIZE;
    pcReaders = (LPWSTR)malloc(sizeof(char) * MAX_READER_NAME_SIZE);

    rv = SCardStatus(hCard, pcReaders, &dwReaderLen, &dwState,
                     &dwProt, pbAtr, &dwAtrLen);

    // Sprawdzansko
    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        free(mszReaders);
        free(pcReaders);
        printf("failed\n");
        return -1;
    }
    else printf("success\n");

    printf("Reader name : %ls\n", pcReaders);
    printf("Reader state : %lx\n", dwState);
    printf("Reader protocol : %lx\n", dwProt - 1);
    printf("Reader ATR size : %lu\n", dwAtrLen);
    printf("Reader ATR value : ");



    // Rozpocznij polaczenie
    for (i = 0; i < dwAtrLen; i++)
    {
        printf("%02X ", pbAtr[i]);
    }
    printf("\n");

    printf("SCardBeginTransaction : ");
    rv = SCardBeginTransaction(hCard);

    // Sprawdzanko
    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");



    // Wybierz folder TELECOM
    BYTE SELECT_TELECOM[] = { 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x7F, 0x10 };
    printf("SCardTransmit : ");
    dwRespLen = 30;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, SELECT_TELECOM,
                       7, nullptr, pbResp1, &dwRespLen);

    // Sprawdzansko
    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("Response APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp1[i]);
    }
    printf("\n");


    BYTE GET_RESPONSE[] = { 0xA0, 0xC0, 0x00, 0x00, 0x1A};
    printf("SCardTransmit : ");
    dwRespLen = 30;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, GET_RESPONSE,
                       5, nullptr, pbResp2, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("Response APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp2[i]);
    }
    printf("\n");




    // Wybierz plik SMS
    BYTE SELECT_SMS[] = { 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x6F, 0x3C };
    printf("SCardTransmit : ");
    dwRespLen = 30;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, SELECT_SMS,
                       7, nullptr, pbResp3, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("Response APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp3[i]);
    }
    printf("\n");


    BYTE GET_RESPONSE2[] = { 0xA0, 0xC0, 0x00, 0x00, 0x0F };
    printf("SCardTransmit : ");
    dwRespLen = 30;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, GET_RESPONSE2,
                       5, nullptr, pbResp4, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("Response APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp4[i]);
    }
    printf("\n");



    // Wyswietl pierwsza wiadomosc
    BYTE READ_RECORD[] = { 0xA0, 0xB2, 0x01, 0x04, 0xB0 };
    printf("SCardTransmit : ");
    dwRespLen = 178;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, READ_RECORD,
                       5, nullptr, pbResp5, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("\nSMS1\nResponse APDU : ");


    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp5[i]);
    }
    printf("\n");



    // Wyswietl druga wiadomosc
    BYTE READ_RECORD2[] = { 0xA0, 0xB2, 0x02, 0x04, 0xB0 };
    printf("SCardTransmit : ");
    dwRespLen = 178;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, READ_RECORD2,
                       5, nullptr, pbResp5, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("\nSMS2\nResponse APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp5[i]);
    }
    printf("\n");



    // Wyswietl wiadomosc trzecia
    BYTE READ_RECORD3[] = { 0xA0, 0xB2, 0x03, 0x04, 0xB0 };
    printf("SCardTransmit : ");
    dwRespLen = 178;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, READ_RECORD3,
                       5, nullptr, pbResp5, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("\nSMS3\nResponse APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp5[i]);
    }
    printf("\n");


    // Wyswietl wiadomowsc czwarta
    BYTE READ_RECORD4[] = { 0xA0, 0xB2, 0x04, 0x04, 0xB0 };
    printf("SCardTransmit : ");
    dwRespLen = 178;
    rv = SCardTransmit(hCard, SCARD_PCI_T0, READ_RECORD2,
                       5, nullptr, pbResp5, &dwRespLen);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");
    printf("\nSMS4\nResponse APDU : ");

    for (i = 0; i < dwRespLen; i++)
    {
        printf("%02X ", pbResp5[i]);
    }
    printf("\n");



    // Zakoncz polaczenie
    printf("SCardEndTransaction : ");
    rv = SCardEndTransaction(hCard, SCARD_LEAVE_CARD);
    if (rv != SCARD_S_SUCCESS)
    {
        SCardDisconnect(hCard, SCARD_RESET_CARD);
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");

    printf("SCardDisconnect : ");
    rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);

    if (rv != SCARD_S_SUCCESS)
    {
        SCardReleaseContext(hContext);
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");

    printf("SCardReleaseContext : ");
    rv = SCardReleaseContext(hContext);

    if (rv != SCARD_S_SUCCESS)
    {
        printf("failed\n");
        free(mszReaders);
        return -1;
    }
    else printf("success\n");

    return 0;
}
