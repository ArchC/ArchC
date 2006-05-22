/* alarm -- there is no support for signals yet */
/*          we just return "no previously scheduled alarm" */

int
alarm(unsigned int seconds)
{
  return 0;
}
