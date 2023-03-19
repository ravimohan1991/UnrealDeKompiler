#! /bin/sh

# $Id: hash_test.sh,v 1.13 2005/01/12 17:37:05 imipak Exp $

if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  if (echo $ac_n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='    '
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi

test_hash ( ) {
	if test "$#" != "3" ; then
		echo "usage: test_hash id plain expected"
		exit 1
	fi
	
	got=`echo $ac_n "$2$ac_c" | ./driver $1 | tr "[:lower:]" "[:upper:]"`

	want=`echo $ac_n "$3$ac_c" | tr "[:lower:]" "[:upper:]"`
	
	if test "$got" = ""; then
		echo "This algorithm ($1) is not available!"
	else
		if test "$got" != "$want" ; then
			echo "  -- TEST FAILED: $1 \"$2\""
			echo "got \"$got\""
			echo "but expected \"$want\""
			exit 1
		else
			echo $ac_n ".$ac_c"
		fi
	fi

}

echo $ac_n "testing CRC32 $ac_c"
test_hash CRC32 "checksum" 7FBEB02E
echo ""

echo $ac_n "testing CRC32B $ac_c"
test_hash CRC32B "checksum" 9ADF6FDE
echo ""

echo $ac_n "testing MD5 $ac_c"
test_hash MD5 "" D41D8CD98F00B204E9800998ECF8427E
test_hash MD5 a 0CC175B9C0F1B6A831C399E269772661
test_hash MD5 abc 900150983CD24FB0D6963F7D28E17F72
test_hash MD5 "message digest" F96B697D7CB7938D525A2F31AAF161D0
test_hash MD5 abcdefghijklmnopqrstuvwxyz C3FCD3D76192E4007DFB496CCA67E13B
test_hash MD5 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 D174AB98D277D9F5A5611C2C9F419D9F
test_hash MD5 12345678901234567890123456789012345678901234567890123456789012345678901234567890 57EDF4A22BE3C955AC49DA2E2107B67A
echo ""

echo $ac_n "testing SHA1 $ac_c"
test_hash SHA1 abc A9993E364706816ABA3E25717850C26C9CD0D89D
test_hash SHA1 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq 84983E441C3BD26EBAAE4AA1F95129E5E54670F1
test_hash SHA1 `perl -e 'print "a"x1000000'` 34AA973CD4C4DAA4F61EEB2BDBAD27316534016F
echo ""

echo $ac_n "testing HAVAL256 $ac_c"
test_hash HAVAL256 abcdefghijklmnopqrstuvwxyz 72FAD4BDE1DA8C8332FB60561A780E7F504F21547B98686824FC33FC796AFA76
test_hash HAVAL256 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 899397D96489281E9E76D5E65ABAB751F312E06C06C07C9C1D42ABD31BB6A404
echo 

echo $ac_n "testing HAVAL224 $ac_c"
test_hash HAVAL224 "0123456789" EE345C97A58190BF0F38BF7CE890231AA5FCF9862BF8E7BEBBF76789
echo 

echo $ac_n "testing HAVAL192 $ac_c"
test_hash HAVAL192 "HAVAL" 8DA26DDAB4317B392B22B638998FE65B0FBE4610D345CF89
echo 

echo $ac_n "testing HAVAL160 $ac_c"
test_hash HAVAL160 "a" 4DA08F514A7275DBC4CECE4A347385983983A830
echo 

echo $ac_n "testing HAVAL128 $ac_c"
test_hash HAVAL128 "" C68F39913F901F3DDF44C707357A7D70
echo 

echo $ac_n "testing RIPEMD128 $ac_c"
test_hash RIPEMD128 "" cdf26213a150dc3ecb610f18f6b38b46
test_hash RIPEMD128 a 86be7afa339d0fc7cfc785e72f578d33
test_hash RIPEMD128 abc c14a12199c66e4ba84636b0f69144c77
test_hash RIPEMD128 "message digest" 9e327b3d6e523062afc1132d7df9d1b8
test_hash RIPEMD128 abcdefghijklmnopqrstuvwxyz fd2aa607f71dc8f510714922b371834e
test_hash RIPEMD128 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq a1aa0689d0fafa2ddc22e88b49133a06
test_hash RIPEMD128 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 d1e959eb179c911faea4624c60c5c702
test_hash RIPEMD128 12345678901234567890123456789012345678901234567890123456789012345678901234567890 3f45ef194732c2dbb2c4a2c769795fa3
test_hash RIPEMD128 `perl -e 'print "a"x1000000'` 4a7f5723f954eba1216c9d8f6320431f
echo ""

echo $ac_n "testing RIPEMD160 $ac_c"
test_hash RIPEMD160 "" 9C1185A5C5E9FC54612808977EE8F548B2258D31
test_hash RIPEMD160 a 0BDC9D2D256B3EE9DAAE347BE6F4DC835A467FFE
test_hash RIPEMD160 abc 8EB208F7E05D987A9B044A8E98C6B087F15A0BFC
test_hash RIPEMD160 "message digest" 5D0689EF49D2FAE572B881B123A85FFA21595F36
test_hash RIPEMD160 abcdefghijklmnopqrstuvwxyz F71C27109C692C1B56BBDCEB5B9D2865B3708DBC
test_hash RIPEMD160 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq 12A053384A9C0C88E405A06C27DCF49ADA62EB2B
test_hash RIPEMD160 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 B0E20B6E3116640286ED3A87A5713079B21F5189
test_hash RIPEMD160 12345678901234567890123456789012345678901234567890123456789012345678901234567890 9b752e45573d4b39f4dbd3323cab82bf63326bfb
test_hash RIPEMD160 `perl -e 'print "a"x1000000'` 52783243c1697bdbe16d37f97f68f08325dc1528
echo ""

echo $ac_n "testing RIPEMD256 $ac_c"
test_hash RIPEMD256 "" 02ba4c4e5f8ecd1877fc52d64d30e37a2d9774fb1e5d026380ae0168e3c5522d
test_hash RIPEMD256 a f9333e45d857f5d90a91bab70a1eba0cfb1be4b0783c9acfcd883a9134692925
test_hash RIPEMD256 abc afbd6e228b9d8cbbcef5ca2d03e6dba10ac0bc7dcbe4680e1e42d2e975459b65
test_hash RIPEMD256 "message digest" 87e971759a1ce47a514d5c914c392c9018c7c46bc14465554afcdf54a5070c0e
test_hash RIPEMD256 abcdefghijklmnopqrstuvwxyz 649d3034751ea216776bf9a18acc81bc7896118a5197968782dd1fd97d8d5133
test_hash RIPEMD256 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq 3843045583aac6c8c8d9128573e7a9809afb2a0f34ccc36ea9e72f16f6368e3f
test_hash RIPEMD256 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 5740a408ac16b720b84424ae931cbb1fe363d1d0bf4017f1a89f7ea6de77a0b8
test_hash RIPEMD256 12345678901234567890123456789012345678901234567890123456789012345678901234567890 06fdcc7a409548aaf91368c06a6275b553e3f099bf0ea4edfd6778df89a890dd
test_hash RIPEMD256 `perl -e 'print "a"x1000000'` ac953744e10e31514c150d4d8d7b677342e33399788296e43ae4850ce4f97978
echo ""

echo $ac_n "testing RIPEMD320 $ac_c"
test_hash RIPEMD320 "" 22d65d5661536cdc75c1fdf5c6de7b41b9f27325ebc61e8557177d705a0ec880151c3a32a00899b8
test_hash RIPEMD320 a ce78850638f92658a5a585097579926dda667a5716562cfcf6fbe77f63542f99b04705d6970dff5d
test_hash RIPEMD320 abc de4c01b3054f8930a79d09ae738e92301e5a17085beffdc1b8d116713e74f82fa942d64cdbc4682d
test_hash RIPEMD320 "message digest" 3a8e28502ed45d422f68844f9dd316e7b98533fa3f2a91d29f84d425c88d6b4eff727df66a7c0197
test_hash RIPEMD320 abcdefghijklmnopqrstuvwxyz cabdb1810b92470a2093aa6bce05952c28348cf43ff60841975166bb40ed234004b8824463e6b009
test_hash RIPEMD320 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq d034a7950cf722021ba4b84df769a5de2060e259df4c9bb4a4268c0e935bbc7470a969c9d072a1ac
test_hash RIPEMD320 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ed544940c86d67f250d232c30b7b3e5770e0c60c8cb9a4cafe3b11388af9920e1b99230b843c86a4
test_hash RIPEMD320 12345678901234567890123456789012345678901234567890123456789012345678901234567890 557888af5f6d8ed62ab66945c6d2a0a47ecd5341e915eb8fea1d0524955f825dc717e4a008ab2d42
test_hash RIPEMD320 `perl -e 'print "a"x1000000'` bdee37f4371e20646b8b0d862dda16292ae36f40965e8c8509e63d1dbddecc503e2b63eb9245bb66
echo ""

echo $ac_n "testing TIGER $ac_c"
test_hash TIGER "" 24F0130C63AC933216166E76B1BB925FF373DE2D49584E7A
test_hash TIGER abc F258C1E88414AB2A527AB541FFC5B8BF935F7B951C132951
test_hash TIGER Tiger 9F00F599072300DD276ABB38C8EB6DEC37790C116F9D2BDF
test_hash TIGER ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+- 87FB2A9083851CF7470D2CF810E6DF9EB586445034A5A386
test_hash TIGER "ABCDEFGHIJKLMNOPQRSTUVWXYZ=abcdefghijklmnopqrstuvwxyz+0123456789" 467DB80863EBCE488DF1CD1261655DE957896565975F9197
test_hash TIGER "Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham" 0C410A042968868A1671DA5A3FD29A725EC1E457D3CDB303
test_hash TIGER "Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham, proceedings of Fast Software Encryption 3, Cambridge." EBF591D5AFA655CE7F22894FF87F54AC89C811B6B0DA3193
test_hash TIGER "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-" 00B83EB4E53440C576AC6AAEE0A7485825FD15E70A59FFE4
echo ""

echo $ac_n "testing TIGER160 $ac_c"
test_hash TIGER160 "" 24F0130C63AC933216166E76B1BB925FF373DE2D
test_hash TIGER160 abc F258C1E88414AB2A527AB541FFC5B8BF935F7B95
test_hash TIGER160 Tiger 9F00F599072300DD276ABB38C8EB6DEC37790C11
test_hash TIGER160 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+- 87FB2A9083851CF7470D2CF810E6DF9EB5864450
test_hash TIGER160 "ABCDEFGHIJKLMNOPQRSTUVWXYZ=abcdefghijklmnopqrstuvwxyz+0123456789" 467DB80863EBCE488DF1CD1261655DE957896565
echo ""

echo $ac_n "testing TIGER128 $ac_c"
test_hash TIGER128 "" 24F0130C63AC933216166E76B1BB925F
test_hash TIGER128 abc F258C1E88414AB2A527AB541FFC5B8BF
test_hash TIGER128 Tiger 9F00F599072300DD276ABB38C8EB6DEC
echo ""

echo $ac_n "testing GOST $ac_c"
test_hash GOST "This is message, length=32 bytes" B1C466D37519B82E8319819FF32595E047A28CB6F83EFF1C6916A815A637FFFA
test_hash GOST "Suppose the original message has length = 50 bytes" 471ABA57A60A770D3A76130635C1FBEA4EF14DE51F78B4AE57DD893B62F55208
echo ""

echo $ac_n "testing MD4 $ac_c"
test_hash MD4 "" 31D6CFE0D16AE931B73C59D7E0C089C0
test_hash MD4 a BDE52CB31DE33E46245E05FBDBD6FB24
test_hash MD4 abc A448017AAF21D8525FC10AE87AA6729D
test_hash MD4 "message digest" D9130A8164549FE818874806E1C7014B
test_hash MD4 abcdefghijklmnopqrstuvwxyz D79E1C308AA5BBCDEEA8ED63DF412DA9
test_hash MD4 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 043F8582F241DB351CE627E153E7F0E4
test_hash MD4 12345678901234567890123456789012345678901234567890123456789012345678901234567890 E33B4DDC9C38F2199C3E7B164FCC0536
echo ""

echo $ac_n "testing SHA256 $ac_c"
test_hash SHA256 abc ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
test_hash SHA256 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq 248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1
test_hash SHA256 abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1
test_hash SHA256 `perl -e 'print "a"x1000000'` cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0
echo ""

echo $ac_n "testing SHA224 $ac_c"
test_hash SHA224 abc 23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7
test_hash SHA224 abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq 75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525
test_hash SHA224 `perl -e 'print "a"x1000000'` 20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67
echo ""

echo $ac_n "testing SHA512 $ac_c"
test_hash SHA512 abc ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f
test_hash SHA512 abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu 8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909
test_hash SHA512 `perl -e 'print "a"x1000000'` e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b
echo ""

echo $ac_n "testing SHA384 $ac_c"
test_hash SHA384 abc cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7
test_hash SHA384 abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu 09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039
test_hash SHA384 `perl -e 'print "a"x1000000'` 9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985
echo ""

echo $ac_n "testing WHIRLPOOL $ac_c"
test_hash WHIRLPOOL "" 19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3
test_hash WHIRLPOOL a 8aca2602792aec6f11a67206531fb7d7f0dff59413145e6973c45001d0087b42d11bc645413aeff63a42391a39145a591a92200d560195e53b478584fdae231a
test_hash WHIRLPOOL abc 4e2448a4c6f486bb16b6562c73b4020bf3043e3a731bce721ae1b303d97e6d4c7181eebdb6c57e277d0e34957114cbd6c797fc9d95d8b582d225292076d4eef5
test_hash WHIRLPOOL "message digest" 378c84a4126e2dc6e56dcc7458377aac838d00032230f53ce1f5700c0ffb4d3b8421557659ef55c106b4b52ac5a4aaa692ed920052838f3362e86dbd37a8903e
test_hash WHIRLPOOL abcdefghijklmnopqrstuvwxyz f1d754662636ffe92c82ebb9212a484a8d38631ead4238f5442ee13b8054e41b08bf2a9251c30b6a0b8aae86177ab4a6f68f673e7207865d5d9819a3dba4eb3b
test_hash WHIRLPOOL ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 dc37e008cf9ee69bf11f00ed9aba26901dd7c28cdec066cc6af42e40f82f3a1e08eba26629129d8fb7cb57211b9281a65517cc879d7b962142c65f5a7af01467
test_hash WHIRLPOOL 12345678901234567890123456789012345678901234567890123456789012345678901234567890 466ef18babb0154d25b9d38a6414f5c08784372bccb204d6549c4afadb6014294d5bd8df2a6c44e538cd047b2681a51a2c60481e88c5a20b2c2a80cf3a9a083b
test_hash WHIRLPOOL abcdbcdecdefdefgefghfghighijhijk 2a987ea40f917061f5d6f0a0e4644f488a7a5a52deee656207c562f988e95c6916bdc8031bc5be1b7b947639fe050b56939baaa0adff9ae6745b7b181c3be3fd
test_hash WHIRLPOOL `perl -e 'print "a"x1000000'` 0C99005BEB57EFF50A7CF005560DDF5D29057FD86B20BFD62DECA0F1CCEA4AF51FC15490EDDC47AF32BB2B66C34FF9AD8C6008AD677F77126953B226E4ED8B01
echo ""

echo $ac_n "testing SNEFRU128 $ac_c"
# Note: the test vectors included in Ralph Merkle's reference code cannot be 
# used here as all of them are \n terminated and this script doesn't support 
# nonprintable characters in test vectors. The last six vectors have been 
# taken from "x86 hash optimization toolkit"'s test vector file.
#test_hash SNEFRU128 "\n" d9fcb3171c097fbba8c8f12aa0906bad
#test_hash SNEFRU128 "1\n" 44ec420ce99c1f62feb66c53c24ae453
#test_hash SNEFRU128 "12\n" 7182051aa852ef6fba4b6c9c9b79b317
#test_hash SNEFRU128 "123\n" bc3a50af82bf56d6a64732bc7b050a93
#test_hash SNEFRU128 "1234\n" c5b8a04985a8eadfb4331a8988752b77
#test_hash SNEFRU128 "12345\n" d559a2b62f6f44111324f85208723707
#test_hash SNEFRU128 "123456\n" 6cfb5e8f1da02bd167b01e4816686c30
#test_hash SNEFRU128 "1234567\n" 29aa48325f275a8a7a01ba1543c54ba5
#test_hash SNEFRU128 "12345678\n" be862a6b68b7df887ebe00319cbc4a47
#test_hash SNEFRU128 "123456789\n" 6103721ccd8ad565d68e90b0f8906163
test_hash SNEFRU128 abc 553d0648928299a0f22a275a02c83b10
test_hash SNEFRU128 abcdefghijklmnopqrstuvwxyz 7840148a66b91c219c36f127a0929606
test_hash SNEFRU128 12345678901234567890123456789012345678901234567890123456789012345678901234567890 d9204ed80bb8430c0b9c244fe485814a
test_hash SNEFRU128 "Test message for buffer workflow test(47 bytes)" dd0d1ab288c3c36671044f41c5077ad6
test_hash SNEFRU128 "Test message for buffer workflow test(48 bytes)." e7054f05bd72d7e86a052153a17c741d
test_hash SNEFRU128 "Test message for buffer workflow test(49 bytes).." 9b34204833422df13c83e10a0c6d080a
echo ""

echo $ac_n "testing SNEFRU256 $ac_c"
# See note above!
#test_hash SNEFRU256 "\n" 2e02687f0d45d5b9b50cb68c3f33e6843d618a1aca2d06893d3eb4e3026b5732
#test_hash SNEFRU256 "1\n" bfea4a05a2a2ef15c736d114598a20b9d9bd4d66b661e6b05ecf6a7737bdc58c
#test_hash SNEFRU256 "12\n" ac677d69761ade3f189c7aef106d5fe7392d324e19cc76d5db4a2c05f2cc2cc5
#test_hash SNEFRU256 "123\n" 061c76aa1db4a22c0e42945e26c48499b5400162e08c640be05d3c007c44793d
#test_hash SNEFRU256 "1234\n" 1e87fe1d9c927e9e24be85e3cc73359873541640a6261793ce5a974953113f5e
#test_hash SNEFRU256 "12345\n" 1b59927d85a9349a87796620fe2ff401a06a7ba48794498ebab978efc3a68912
#test_hash SNEFRU256 "123456\n" 28e9d9bc35032b68faeda88101ecb2524317e9da111b0e3e7094107212d9cf72
#test_hash SNEFRU256 "1234567\n" f7fff4ee74fd1b8d6b3267f84e47e007f029d13b8af7e37e34d13b469b8f248f
#test_hash SNEFRU256 "12345678\n" ee7d64b0102b2205e98926613b200185559d08be6ad787da717c968744e11af3
#test_hash SNEFRU256 "123456789\n" 4ca72639e40e9ab9c0c3f523c4449b3911632d374c124d7702192ec2e4e0b7a3
test_hash SNEFRU256 abc 7d033205647a2af3dc8339f6cb25643c33ebc622d32979c4b612b02c4903031b
test_hash SNEFRU256 abcdefghijklmnopqrstuvwxyz 9304bb2f876d9c4f54546cf7ec59e0a006bead745f08c642f25a7c808e0bf86e
test_hash SNEFRU256 12345678901234567890123456789012345678901234567890123456789012345678901234567890 d5fce38a152a2d9b83ab44c29306ee45ab0aed0e38c957ec431dab6ed6bb71b8
echo ""

echo $ac_n "testing MD2 $ac_c"
test_hash MD2 "" 8350e5a3e24c153df2275c9f80692773
test_hash MD2 a 32ec01ec4a6dac72c0ab96fb34c0b5d1
test_hash MD2 abc da853b0d3f88d99b30283a69e6ded6bb
test_hash MD2 "message digest" ab4f496bfb2a530b219ff33031fe06b0
test_hash MD2 abcdefghijklmnopqrstuvwxyz 4e8ddff3650292ab5a4108c3aa47940b
test_hash MD2 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 da33def2a42df13975352846c30338cd
test_hash MD2 12345678901234567890123456789012345678901234567890123456789012345678901234567890 d5976f79d83d3a0dc9806c3c66f3efd8
echo ""



echo "everything seems to be fine :-)"

exit 0
