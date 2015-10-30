#include <cstdio>
#include <iostream>
#include "PF.h"
#include "PF_HashTable.h"

using namespace std;

RC PrintFile(PF_FileHandle &fh) {
    PF_PageHandle ph;
    char* data;
    PageNum pageNum, temp;

    if (fh.GetFirstPage(ph))
        puts("error7");
    do {
        if (ph.GetData(data) || ph.GetPageNum(pageNum))
            puts("error8");
        memcpy(&temp, data, sizeof(PageNum));
        cout << "page data: " << temp << " " << pageNum << endl;
        if (fh.UnpinPage(pageNum)) {
            puts("error8.5");
        }
    } while (!fh.GetNextPage(pageNum, ph));
    if (fh.GetNextPage(pageNum, ph) != PF_EOF)
        puts("error9");
    return RC_OK;
}

RC WriteFile(PF_Manager &pfm, char *fileName) {
    PF_FileHandle fh;
    PF_PageHandle ph;
    char *data;
    PageNum pageNum;
    RC rc;
    if (pfm.OpenFile(fileName, fh))
        puts("error1!");

    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        if (fh.AllocatePage(ph) || ph.GetData(data) || ph.GetPageNum(pageNum))
            puts("error2!");
        if (i != pageNum) {
            cout << "pageNum incorrect " << pageNum << " " << i << endl;
        }
        memcpy(data, &pageNum, sizeof(pageNum));
        cout << "page allocated: " << pageNum << endl;
    }

    if (fh.AllocatePage(ph) != PF_NO_BUFFER) {
        puts("error3!");
    }

    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        if (fh.UnpinPage(i))
            puts("error4!");
    }
    if ((rc = pfm.CloseFile(fh)))
        puts("error5!");
    return RC_OK;
}

RC ReadFile(PF_Manager &pfm, char* fileName) {
    PF_FileHandle fh;
    if (pfm.OpenFile(fileName, fh) || PrintFile(fh) || pfm.CloseFile(fh))
        puts("error6!");
    return RC_OK;
}

RC TestPF() {
    PF_Manager pfm;
    PF_FileHandle fh1, fh2;
    PF_PageHandle ph;
    RC rc;
    char *data;
    PageNum pageNum, temp;

    if (pfm.CreateFile("f1") || pfm.CreateFile("f2") || WriteFile(pfm, "f1") || ReadFile(pfm, "f1") || WriteFile(pfm, "f2") || ReadFile(pfm, "f2") || pfm.OpenFile("f1", fh1) || pfm.OpenFile("f2", fh2))
        puts("error10");
    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        if (i & 1) {
            if (fh1.DisposePage(i)) {
                puts("error11");
            }
        } else {
            if (fh2.DisposePage(i)) {
                puts("erorr12");
            };
        }
    }
    if (fh1.FlushPages() || fh2.FlushPages() || pfm.CloseFile(fh1) || pfm.CloseFile(fh2) || ReadFile(pfm, "f1") || ReadFile(pfm, "f2") || pfm.DestroyFile("f1") || pfm.DestroyFile("f2")) {
        puts("error13");
    }

    if ((rc = pfm.CreateFile("f1")) ||
        (rc = pfm.CreateFile("f2")) ||
        (rc = WriteFile(pfm, "f1")) ||
        (rc = WriteFile(pfm, "f2")) ||
        (rc = pfm.OpenFile("f1", fh1)) ||
        (rc = pfm.OpenFile("f2", fh2)))
        puts("error14");

    for (int i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
        if ((rc = fh2.AllocatePage(ph)) ||
            (rc = ph.GetData(data)) ||
            (rc = ph.GetPageNum(pageNum)))
            puts("error15");
        if (i != pageNum) {
            cout << "Page number is incorrect:" << (int)pageNum << " " << i << "\n";
            exit(1);
        }
        memcpy(data, (char*)&pageNum, sizeof(PageNum));
        //  memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &pageNum, sizeof(PageNum));

        if ((rc = fh2.MarkDirty(pageNum)) ||
            (rc = fh2.UnpinPage(pageNum)))
            puts("error16");

        if ((rc = fh1.AllocatePage(ph)) ||
            (rc = ph.GetData(data)) ||
            (rc = ph.GetPageNum(pageNum)))
            puts("error17");

        if (i != pageNum) {
            cout << "Page number is incorrect:" << (int)pageNum << " " << i << "\n";
            exit(1);
        }

        memcpy(data, (char*)&pageNum, sizeof(PageNum));
        // memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &pageNum, sizeof(PageNum));

        if ((rc = fh1.MarkDirty(pageNum)) ||
            (rc = fh1.UnpinPage(pageNum)))
            puts("error18");
    }
    for (int i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
        if (i & 1) {
            if ((rc = fh1.DisposePage(i)))
                puts("error19");
        }
        else
        if ((rc = fh2.DisposePage(i)))
            puts("error20");
    }
    for (int i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
        if (i & 1) {
            if ((rc = fh2.GetThisPage(i, ph)) ||
                (rc = ph.GetData(data)) ||
                (rc = ph.GetPageNum(pageNum)))
                puts("error21");

            memcpy((char *)&temp, data, sizeof(PageNum));

            cout << "Page: " << (int)pageNum << " " << (int)temp << "\n";

            if ((rc = fh2.UnpinPage(i)))
                puts("error22");
        }
    }

    for (int i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
        if (!(i & 1)) {
            if ((rc = fh1.GetThisPage(i, ph)) ||
                (rc = ph.GetData(data)) ||
                (rc = ph.GetPageNum(pageNum)))
                return(rc);

            memcpy((char *)&temp, data, sizeof(PageNum));

            cout << "Page: " << (int)pageNum << " " << (int)temp << "\n";

            if ((rc = fh1.UnpinPage(i)))
                puts("error23");
        }
    }

    cout << "Printing file 2, then file 1\n";

    if ((rc = PrintFile(fh2)) ||
        (rc = PrintFile(fh1)))
        puts("error24");

    cout << "Putting stuff into the holes of file 1\n";

    for (int i = 0; i < PF_BUFFER_SIZE / 2; i++) {
        if ((rc = fh1.AllocatePage(ph)) ||
            (rc = ph.GetData(data)) ||
            (rc = ph.GetPageNum(pageNum)))
            puts("error25");

        memcpy(data, (char *)&pageNum, sizeof(PageNum));
        // memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &pageNum, sizeof(PageNum));

        if ((rc = fh1.MarkDirty(pageNum)) ||
            (rc = fh1.UnpinPage(pageNum)))
            puts("error26");
    }

    cout << "Print file 1 and then close both files\n";

    if ((rc = PrintFile(fh1)) ||
        (rc = pfm.CloseFile(fh1)) ||
        (rc = pfm.CloseFile(fh2)))
        puts("error27");

    cout << "Reopen file 1 and test some error conditions\n";

    if ((rc = pfm.OpenFile("F1", fh1)))
        puts("error28");

    //  if ((rc = pfm.DestroyFile(FILE1)) != PF_FILEOPEN) {
    //    cout << "Destroy file while open should fail: ";
    //    return(rc);
    //  }

    if ((rc = fh1.DisposePage(100)) != PF_INVALID_PAGE) {
        cout << "Dispose invalid page should fail: ";
        puts("error29");
    }

    // Get page 1

    if ((rc = fh1.GetThisPage(1, ph)))
        puts("error30");

    if ((rc = fh1.DisposePage(1)) != PF_PAGE_PINNED) {
        cout << "Dispose pinned page should fail: ";
        puts("error31");
    }

    if ((rc = ph.GetData(data)) ||
        (rc = ph.GetPageNum(pageNum)))
        puts("error32");

    memcpy((char *)&temp, data, sizeof(PageNum));

    if (temp != 1 || pageNum != 1) {
        cout << "Asked for page 1, got: " << (int)pageNum << " " <<
        (int)temp << "\n";
        exit(1);
    }

    if ((rc = fh1.UnpinPage(pageNum)))
        puts("error33");

    if ((rc = fh1.UnpinPage(pageNum)) != PF_PAGE_UNPINNED) {
        cout << "Unpin unpinned page should fail: ";
        puts("error34");
    }

    cout << "Opening file 1 twice, printing out both copies\n";

    if ((rc = pfm.OpenFile("f1", fh2)))
        puts("error35");

    if ((rc = PrintFile(fh1)) ||
        (rc = PrintFile(fh2)))
        puts("error36");

    cout << "Closing and destroying both files\n";

    if ((rc = pfm.CloseFile(fh1)) ||
        (rc = pfm.CloseFile(fh2)) ||
        (rc = pfm.DestroyFile("f1")) ||
        (rc = pfm.DestroyFile("f2")))
        puts("error37");


    return RC_OK;
}

RC TestHash()
{
    PF_HashTable ht(PF_HASH_TABLE_SIZE);
    RC           rc;
    int          i, s;
    PageNum      p;

    cout << "Testing hash table.  Inserting entries\n";

    for (i = 1; i < 11; i++)
        for (p = 1; p < 11; p++)
            if ((rc = ht.Insert((FILE *)i, p, i + p)))
                return(rc);

    cout << "Searching for entries\n";

    for (i = 1; i < 11; i++)
        for (p = 1; p < 11; p++)
            if ((rc = ht.Find((FILE *)i, p, s)))
                return(rc);

    cout << "Deleting entries in reverse order\n";

    for (p = 10; p > 0; p--)
        for (i = 10; i > 0; i--)
            if ((rc = ht.Delete((FILE *)i,p)))
                return(rc);

    cout << "Ensuring all entries were deleted\n";

    for (i = 1; i < 11; i++)
        for (p = 1; p < 11; p++)
            if ((rc = ht.Find((FILE *)i, p, s)) != PF_HASH_NOT_FOUND) {
                cout << "Find deleted hash entry should fail: ";
                return(rc);
            }

    // Return ok
    return (0);
}

int main()
{
    if (TestPF() || TestHash()) {
        puts("error!");
    }
    return 0;
}
