# UnrealDeKompiler
A tool to reconstruct programmer's unreal script

For online API visit [doxy-docs](https://ravimohan1991.github.io/UnrealDeKompiler/)

## Resources for consulting

-  [DCC_decompilation_thesis.pdf](https://github.com/ravimohan1991/UnrealDeKompiler/files/10913819/DCC_decompilation_thesis.pdf)
-  [Summarized Notes](https://www.cs.cmu.edu/~fp/courses/15411-f13/lectures/20-decompilation.pdf)
-  [Marcus Hutchins](https://www.youtube.com/playlist?list=PLPsJIruML_ZivGWUd6bPkwDe-KFOIYg7p)
-  [Talk by Tsviatko](https://www.youtube.com/watch?v=uYZZbteb8Gc)
-  [Wikipedia](https://en.wikipedia.org/wiki/Decompiler)
-  [wxWidgets cmake](https://github.com/Ravbug/UnityHubNative/tree/master)
-  [Vivek's disassembler](https://github.com/vmt/udis86)

## Crash course on gh-pages

To create a branch of your github software repository which contains nothing but 
documentation, first create a new, empty branch on your local copy and push it up.

cd /path/to/repoName

git symbolic-ref HEAD refs/heads/gh-pages

rm .git/index

git clean -fdx

echo "My GitHub Page" > index.md

git add .

git commit -a -m "First pages commit"

git push origin gh-pages

You have to configure your repository - Settings -> Action -> General -> Workflow permissions and choose read and write permissions
