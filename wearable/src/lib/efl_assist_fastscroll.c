#include "efl_assist.h"
#include "efl_assist_private.h"

static char *non_latin_lan[] = {
     "ar_AE.UTF-8",
     "ar_IL.UTF-8",
     "as_IN.UTF-8",
     "bg_BG.UTF-8",
     "bn_IN.UTF-8",
     "el_GR.UTF-8",
     "fa_IR.UTF-8",
     "gu_IN.UTF-8",
     "he_IL.UTF-8",
     "hi_IN.UTF-8",
     "hy_AM.UTF-8",
     "ja_JP.UTF-8",
     "ka_GE.UTF-8",
     "kk_KZ.UTF-8",
     "km_KH.UTF-8",
     "kn_CA.UTF-8",
     "ko_KR.UTF-8",
     "lo_LA.UTF-8",
     "mk_MK.UTF-8",
     "ml_MY.UTF-8",
     "mr_IN.UTF-8",
     "ne_NP.UTF-8",
     "or_IN.UTF-8",
     "pa_PK.UTF-8",
     "ru_RU.UTF-8",
     "si_LK.UTF-8",
     "ta_IN.UTF-8",
     "te_IN.UTF-8",
     "th_TH.UTF-8",
     "uk_UA.UTF-8",
     "ur_PK.UTF-8",
     "zh_TW.UTF-8",
     NULL
};

EAPI Eina_Bool
ea_locale_latin_get(const char *locale)
{
   if (!locale) return EINA_FALSE;

   int i = 0;

   while(non_latin_lan[i])
     {
        if (!strcmp(non_latin_lan[i], locale)) return EINA_FALSE;
        i++;
     }
   return EINA_TRUE;
}
