/*************************************************************************
	> File Name: main.cpp
	> Author:    orange
	> Mail:      huiyi950512@gmail.com
	> Created Time: 2018年06月26日 星期二 15时45分24秒
 ************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <string>
#include "consistent_hash.h"

using namespace std;

int main()
{
    ConsistentHash *con = new ConsistentHash;
    con->InitHashRing();

    set<string> myset;

    string openid("o04IBAEj-4ep7LOSdHNkDCRBIV6I");
    myset = con->GetNodeSet(openid);

    for(auto it = myset.begin();it != myset.end();it++)
    {
        cout << *it << endl;
    }

}
