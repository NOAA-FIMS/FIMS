# `get_estimates()` works with deterministic run

    Code
      print(dplyr::select(get_estimates(deterministic_results), -estimated, -expected,
      -uncertainty, -gradient, -likelihood, -log_like_cv, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 19,816 x 19
          module_name module_id module_type label            type   type_id
          <chr>           <int> <chr>       <chr>            <chr>    <int>
        1 Fleet               1 <NA>        log_Fmort        vector       4
        2 Fleet               1 <NA>        log_Fmort        vector       4
        3 Fleet               1 <NA>        log_Fmort        vector       4
        4 Fleet               1 <NA>        log_Fmort        vector       4
        5 Fleet               1 <NA>        log_Fmort        vector       4
        6 Fleet               1 <NA>        log_Fmort        vector       4
        7 Fleet               1 <NA>        log_Fmort        vector       4
        8 Fleet               1 <NA>        log_Fmort        vector       4
        9 Fleet               1 <NA>        log_Fmort        vector       4
       10 Fleet               1 <NA>        log_Fmort        vector       4
       11 Fleet               1 <NA>        log_Fmort        vector       4
       12 Fleet               1 <NA>        log_Fmort        vector       4
       13 Fleet               1 <NA>        log_Fmort        vector       4
       14 Fleet               1 <NA>        log_Fmort        vector       4
       15 Fleet               1 <NA>        log_Fmort        vector       4
       16 Fleet               1 <NA>        log_Fmort        vector       4
       17 Fleet               1 <NA>        log_Fmort        vector       4
       18 Fleet               1 <NA>        log_Fmort        vector       4
       19 Fleet               1 <NA>        log_Fmort        vector       4
       20 Fleet               1 <NA>        log_Fmort        vector       4
       21 Fleet               1 <NA>        log_Fmort        vector       4
       22 Fleet               1 <NA>        log_Fmort        vector       4
       23 Fleet               1 <NA>        log_Fmort        vector       4
       24 Fleet               1 <NA>        log_Fmort        vector       4
       25 Fleet               1 <NA>        log_Fmort        vector       4
       26 Fleet               1 <NA>        log_Fmort        vector       4
       27 Fleet               1 <NA>        log_Fmort        vector       4
       28 Fleet               1 <NA>        log_Fmort        vector       4
       29 Fleet               1 <NA>        log_Fmort        vector       4
       30 Fleet               1 <NA>        log_Fmort        vector       4
       31 Fleet               1 <NA>        log_q            vector       3
       32 Fleet               1 <NA>        agecomp_expected <NA>        NA
       33 Fleet               1 <NA>        agecomp_expected <NA>        NA
       34 Fleet               1 <NA>        agecomp_expected <NA>        NA
       35 Fleet               1 <NA>        agecomp_expected <NA>        NA
       36 Fleet               1 <NA>        agecomp_expected <NA>        NA
       37 Fleet               1 <NA>        agecomp_expected <NA>        NA
       38 Fleet               1 <NA>        agecomp_expected <NA>        NA
       39 Fleet               1 <NA>        agecomp_expected <NA>        NA
       40 Fleet               1 <NA>        agecomp_expected <NA>        NA
       41 Fleet               1 <NA>        agecomp_expected <NA>        NA
       42 Fleet               1 <NA>        agecomp_expected <NA>        NA
       43 Fleet               1 <NA>        agecomp_expected <NA>        NA
       44 Fleet               1 <NA>        agecomp_expected <NA>        NA
       45 Fleet               1 <NA>        agecomp_expected <NA>        NA
       46 Fleet               1 <NA>        agecomp_expected <NA>        NA
       47 Fleet               1 <NA>        agecomp_expected <NA>        NA
       48 Fleet               1 <NA>        agecomp_expected <NA>        NA
       49 Fleet               1 <NA>        agecomp_expected <NA>        NA
       50 Fleet               1 <NA>        agecomp_expected <NA>        NA
       51 Fleet               1 <NA>        agecomp_expected <NA>        NA
       52 Fleet               1 <NA>        agecomp_expected <NA>        NA
       53 Fleet               1 <NA>        agecomp_expected <NA>        NA
       54 Fleet               1 <NA>        agecomp_expected <NA>        NA
       55 Fleet               1 <NA>        agecomp_expected <NA>        NA
       56 Fleet               1 <NA>        agecomp_expected <NA>        NA
       57 Fleet               1 <NA>        agecomp_expected <NA>        NA
       58 Fleet               1 <NA>        agecomp_expected <NA>        NA
       59 Fleet               1 <NA>        agecomp_expected <NA>        NA
       60 Fleet               1 <NA>        agecomp_expected <NA>        NA
       61 Fleet               1 <NA>        agecomp_expected <NA>        NA
       62 Fleet               1 <NA>        agecomp_expected <NA>        NA
       63 Fleet               1 <NA>        agecomp_expected <NA>        NA
       64 Fleet               1 <NA>        agecomp_expected <NA>        NA
       65 Fleet               1 <NA>        agecomp_expected <NA>        NA
       66 Fleet               1 <NA>        agecomp_expected <NA>        NA
       67 Fleet               1 <NA>        agecomp_expected <NA>        NA
       68 Fleet               1 <NA>        agecomp_expected <NA>        NA
       69 Fleet               1 <NA>        agecomp_expected <NA>        NA
       70 Fleet               1 <NA>        agecomp_expected <NA>        NA
       71 Fleet               1 <NA>        agecomp_expected <NA>        NA
       72 Fleet               1 <NA>        agecomp_expected <NA>        NA
       73 Fleet               1 <NA>        agecomp_expected <NA>        NA
       74 Fleet               1 <NA>        agecomp_expected <NA>        NA
       75 Fleet               1 <NA>        agecomp_expected <NA>        NA
       76 Fleet               1 <NA>        agecomp_expected <NA>        NA
       77 Fleet               1 <NA>        agecomp_expected <NA>        NA
       78 Fleet               1 <NA>        agecomp_expected <NA>        NA
       79 Fleet               1 <NA>        agecomp_expected <NA>        NA
       80 Fleet               1 <NA>        agecomp_expected <NA>        NA
       81 Fleet               1 <NA>        agecomp_expected <NA>        NA
       82 Fleet               1 <NA>        agecomp_expected <NA>        NA
       83 Fleet               1 <NA>        agecomp_expected <NA>        NA
       84 Fleet               1 <NA>        agecomp_expected <NA>        NA
       85 Fleet               1 <NA>        agecomp_expected <NA>        NA
       86 Fleet               1 <NA>        agecomp_expected <NA>        NA
       87 Fleet               1 <NA>        agecomp_expected <NA>        NA
       88 Fleet               1 <NA>        agecomp_expected <NA>        NA
       89 Fleet               1 <NA>        agecomp_expected <NA>        NA
       90 Fleet               1 <NA>        agecomp_expected <NA>        NA
       91 Fleet               1 <NA>        agecomp_expected <NA>        NA
       92 Fleet               1 <NA>        agecomp_expected <NA>        NA
       93 Fleet               1 <NA>        agecomp_expected <NA>        NA
       94 Fleet               1 <NA>        agecomp_expected <NA>        NA
       95 Fleet               1 <NA>        agecomp_expected <NA>        NA
       96 Fleet               1 <NA>        agecomp_expected <NA>        NA
       97 Fleet               1 <NA>        agecomp_expected <NA>        NA
       98 Fleet               1 <NA>        agecomp_expected <NA>        NA
       99 Fleet               1 <NA>        agecomp_expected <NA>        NA
      100 Fleet               1 <NA>        agecomp_expected <NA>        NA
      101 Fleet               1 <NA>        agecomp_expected <NA>        NA
      102 Fleet               1 <NA>        agecomp_expected <NA>        NA
      103 Fleet               1 <NA>        agecomp_expected <NA>        NA
      104 Fleet               1 <NA>        agecomp_expected <NA>        NA
      105 Fleet               1 <NA>        agecomp_expected <NA>        NA
      106 Fleet               1 <NA>        agecomp_expected <NA>        NA
      107 Fleet               1 <NA>        agecomp_expected <NA>        NA
      108 Fleet               1 <NA>        agecomp_expected <NA>        NA
      109 Fleet               1 <NA>        agecomp_expected <NA>        NA
      110 Fleet               1 <NA>        agecomp_expected <NA>        NA
      111 Fleet               1 <NA>        agecomp_expected <NA>        NA
      112 Fleet               1 <NA>        agecomp_expected <NA>        NA
      113 Fleet               1 <NA>        agecomp_expected <NA>        NA
      114 Fleet               1 <NA>        agecomp_expected <NA>        NA
      115 Fleet               1 <NA>        agecomp_expected <NA>        NA
      116 Fleet               1 <NA>        agecomp_expected <NA>        NA
      117 Fleet               1 <NA>        agecomp_expected <NA>        NA
      118 Fleet               1 <NA>        agecomp_expected <NA>        NA
      119 Fleet               1 <NA>        agecomp_expected <NA>        NA
      120 Fleet               1 <NA>        agecomp_expected <NA>        NA
      121 Fleet               1 <NA>        agecomp_expected <NA>        NA
      122 Fleet               1 <NA>        agecomp_expected <NA>        NA
      123 Fleet               1 <NA>        agecomp_expected <NA>        NA
      124 Fleet               1 <NA>        agecomp_expected <NA>        NA
      125 Fleet               1 <NA>        agecomp_expected <NA>        NA
      126 Fleet               1 <NA>        agecomp_expected <NA>        NA
      127 Fleet               1 <NA>        agecomp_expected <NA>        NA
      128 Fleet               1 <NA>        agecomp_expected <NA>        NA
      129 Fleet               1 <NA>        agecomp_expected <NA>        NA
      130 Fleet               1 <NA>        agecomp_expected <NA>        NA
      131 Fleet               1 <NA>        agecomp_expected <NA>        NA
      132 Fleet               1 <NA>        agecomp_expected <NA>        NA
      133 Fleet               1 <NA>        agecomp_expected <NA>        NA
      134 Fleet               1 <NA>        agecomp_expected <NA>        NA
      135 Fleet               1 <NA>        agecomp_expected <NA>        NA
      136 Fleet               1 <NA>        agecomp_expected <NA>        NA
      137 Fleet               1 <NA>        agecomp_expected <NA>        NA
      138 Fleet               1 <NA>        agecomp_expected <NA>        NA
      139 Fleet               1 <NA>        agecomp_expected <NA>        NA
      140 Fleet               1 <NA>        agecomp_expected <NA>        NA
      141 Fleet               1 <NA>        agecomp_expected <NA>        NA
      142 Fleet               1 <NA>        agecomp_expected <NA>        NA
      143 Fleet               1 <NA>        agecomp_expected <NA>        NA
      144 Fleet               1 <NA>        agecomp_expected <NA>        NA
      145 Fleet               1 <NA>        agecomp_expected <NA>        NA
      146 Fleet               1 <NA>        agecomp_expected <NA>        NA
      147 Fleet               1 <NA>        agecomp_expected <NA>        NA
      148 Fleet               1 <NA>        agecomp_expected <NA>        NA
      149 Fleet               1 <NA>        agecomp_expected <NA>        NA
      150 Fleet               1 <NA>        agecomp_expected <NA>        NA
      151 Fleet               1 <NA>        agecomp_expected <NA>        NA
      152 Fleet               1 <NA>        agecomp_expected <NA>        NA
      153 Fleet               1 <NA>        agecomp_expected <NA>        NA
      154 Fleet               1 <NA>        agecomp_expected <NA>        NA
      155 Fleet               1 <NA>        agecomp_expected <NA>        NA
      156 Fleet               1 <NA>        agecomp_expected <NA>        NA
      157 Fleet               1 <NA>        agecomp_expected <NA>        NA
      158 Fleet               1 <NA>        agecomp_expected <NA>        NA
      159 Fleet               1 <NA>        agecomp_expected <NA>        NA
      160 Fleet               1 <NA>        agecomp_expected <NA>        NA
      161 Fleet               1 <NA>        agecomp_expected <NA>        NA
      162 Fleet               1 <NA>        agecomp_expected <NA>        NA
      163 Fleet               1 <NA>        agecomp_expected <NA>        NA
      164 Fleet               1 <NA>        agecomp_expected <NA>        NA
      165 Fleet               1 <NA>        agecomp_expected <NA>        NA
      166 Fleet               1 <NA>        agecomp_expected <NA>        NA
      167 Fleet               1 <NA>        agecomp_expected <NA>        NA
      168 Fleet               1 <NA>        agecomp_expected <NA>        NA
      169 Fleet               1 <NA>        agecomp_expected <NA>        NA
      170 Fleet               1 <NA>        agecomp_expected <NA>        NA
      171 Fleet               1 <NA>        agecomp_expected <NA>        NA
      172 Fleet               1 <NA>        agecomp_expected <NA>        NA
      173 Fleet               1 <NA>        agecomp_expected <NA>        NA
      174 Fleet               1 <NA>        agecomp_expected <NA>        NA
      175 Fleet               1 <NA>        agecomp_expected <NA>        NA
      176 Fleet               1 <NA>        agecomp_expected <NA>        NA
      177 Fleet               1 <NA>        agecomp_expected <NA>        NA
      178 Fleet               1 <NA>        agecomp_expected <NA>        NA
      179 Fleet               1 <NA>        agecomp_expected <NA>        NA
      180 Fleet               1 <NA>        agecomp_expected <NA>        NA
      181 Fleet               1 <NA>        agecomp_expected <NA>        NA
      182 Fleet               1 <NA>        agecomp_expected <NA>        NA
      183 Fleet               1 <NA>        agecomp_expected <NA>        NA
      184 Fleet               1 <NA>        agecomp_expected <NA>        NA
      185 Fleet               1 <NA>        agecomp_expected <NA>        NA
      186 Fleet               1 <NA>        agecomp_expected <NA>        NA
      187 Fleet               1 <NA>        agecomp_expected <NA>        NA
      188 Fleet               1 <NA>        agecomp_expected <NA>        NA
      189 Fleet               1 <NA>        agecomp_expected <NA>        NA
      190 Fleet               1 <NA>        agecomp_expected <NA>        NA
      191 Fleet               1 <NA>        agecomp_expected <NA>        NA
      192 Fleet               1 <NA>        agecomp_expected <NA>        NA
      193 Fleet               1 <NA>        agecomp_expected <NA>        NA
      194 Fleet               1 <NA>        agecomp_expected <NA>        NA
      195 Fleet               1 <NA>        agecomp_expected <NA>        NA
      196 Fleet               1 <NA>        agecomp_expected <NA>        NA
      197 Fleet               1 <NA>        agecomp_expected <NA>        NA
      198 Fleet               1 <NA>        agecomp_expected <NA>        NA
      199 Fleet               1 <NA>        agecomp_expected <NA>        NA
      200 Fleet               1 <NA>        agecomp_expected <NA>        NA
      201 Fleet               1 <NA>        agecomp_expected <NA>        NA
      202 Fleet               1 <NA>        agecomp_expected <NA>        NA
      203 Fleet               1 <NA>        agecomp_expected <NA>        NA
      204 Fleet               1 <NA>        agecomp_expected <NA>        NA
      205 Fleet               1 <NA>        agecomp_expected <NA>        NA
      206 Fleet               1 <NA>        agecomp_expected <NA>        NA
      207 Fleet               1 <NA>        agecomp_expected <NA>        NA
      208 Fleet               1 <NA>        agecomp_expected <NA>        NA
      209 Fleet               1 <NA>        agecomp_expected <NA>        NA
      210 Fleet               1 <NA>        agecomp_expected <NA>        NA
      211 Fleet               1 <NA>        agecomp_expected <NA>        NA
      212 Fleet               1 <NA>        agecomp_expected <NA>        NA
      213 Fleet               1 <NA>        agecomp_expected <NA>        NA
      214 Fleet               1 <NA>        agecomp_expected <NA>        NA
      215 Fleet               1 <NA>        agecomp_expected <NA>        NA
      216 Fleet               1 <NA>        agecomp_expected <NA>        NA
      217 Fleet               1 <NA>        agecomp_expected <NA>        NA
      218 Fleet               1 <NA>        agecomp_expected <NA>        NA
      219 Fleet               1 <NA>        agecomp_expected <NA>        NA
      220 Fleet               1 <NA>        agecomp_expected <NA>        NA
      221 Fleet               1 <NA>        agecomp_expected <NA>        NA
      222 Fleet               1 <NA>        agecomp_expected <NA>        NA
      223 Fleet               1 <NA>        agecomp_expected <NA>        NA
      224 Fleet               1 <NA>        agecomp_expected <NA>        NA
      225 Fleet               1 <NA>        agecomp_expected <NA>        NA
      226 Fleet               1 <NA>        agecomp_expected <NA>        NA
      227 Fleet               1 <NA>        agecomp_expected <NA>        NA
      228 Fleet               1 <NA>        agecomp_expected <NA>        NA
      229 Fleet               1 <NA>        agecomp_expected <NA>        NA
      230 Fleet               1 <NA>        agecomp_expected <NA>        NA
      231 Fleet               1 <NA>        agecomp_expected <NA>        NA
      232 Fleet               1 <NA>        agecomp_expected <NA>        NA
      233 Fleet               1 <NA>        agecomp_expected <NA>        NA
      234 Fleet               1 <NA>        agecomp_expected <NA>        NA
      235 Fleet               1 <NA>        agecomp_expected <NA>        NA
      236 Fleet               1 <NA>        agecomp_expected <NA>        NA
      237 Fleet               1 <NA>        agecomp_expected <NA>        NA
      238 Fleet               1 <NA>        agecomp_expected <NA>        NA
      239 Fleet               1 <NA>        agecomp_expected <NA>        NA
      240 Fleet               1 <NA>        agecomp_expected <NA>        NA
      241 Fleet               1 <NA>        agecomp_expected <NA>        NA
      242 Fleet               1 <NA>        agecomp_expected <NA>        NA
      243 Fleet               1 <NA>        agecomp_expected <NA>        NA
      244 Fleet               1 <NA>        agecomp_expected <NA>        NA
      245 Fleet               1 <NA>        agecomp_expected <NA>        NA
      246 Fleet               1 <NA>        agecomp_expected <NA>        NA
      247 Fleet               1 <NA>        agecomp_expected <NA>        NA
      248 Fleet               1 <NA>        agecomp_expected <NA>        NA
      249 Fleet               1 <NA>        agecomp_expected <NA>        NA
      250 Fleet               1 <NA>        agecomp_expected <NA>        NA
      251 Fleet               1 <NA>        agecomp_expected <NA>        NA
      252 Fleet               1 <NA>        agecomp_expected <NA>        NA
      253 Fleet               1 <NA>        agecomp_expected <NA>        NA
      254 Fleet               1 <NA>        agecomp_expected <NA>        NA
      255 Fleet               1 <NA>        agecomp_expected <NA>        NA
      256 Fleet               1 <NA>        agecomp_expected <NA>        NA
      257 Fleet               1 <NA>        agecomp_expected <NA>        NA
      258 Fleet               1 <NA>        agecomp_expected <NA>        NA
      259 Fleet               1 <NA>        agecomp_expected <NA>        NA
      260 Fleet               1 <NA>        agecomp_expected <NA>        NA
      261 Fleet               1 <NA>        agecomp_expected <NA>        NA
      262 Fleet               1 <NA>        agecomp_expected <NA>        NA
      263 Fleet               1 <NA>        agecomp_expected <NA>        NA
      264 Fleet               1 <NA>        agecomp_expected <NA>        NA
      265 Fleet               1 <NA>        agecomp_expected <NA>        NA
      266 Fleet               1 <NA>        agecomp_expected <NA>        NA
      267 Fleet               1 <NA>        agecomp_expected <NA>        NA
      268 Fleet               1 <NA>        agecomp_expected <NA>        NA
      269 Fleet               1 <NA>        agecomp_expected <NA>        NA
      270 Fleet               1 <NA>        agecomp_expected <NA>        NA
      271 Fleet               1 <NA>        agecomp_expected <NA>        NA
      272 Fleet               1 <NA>        agecomp_expected <NA>        NA
      273 Fleet               1 <NA>        agecomp_expected <NA>        NA
      274 Fleet               1 <NA>        agecomp_expected <NA>        NA
      275 Fleet               1 <NA>        agecomp_expected <NA>        NA
      276 Fleet               1 <NA>        agecomp_expected <NA>        NA
      277 Fleet               1 <NA>        agecomp_expected <NA>        NA
      278 Fleet               1 <NA>        agecomp_expected <NA>        NA
      279 Fleet               1 <NA>        agecomp_expected <NA>        NA
      280 Fleet               1 <NA>        agecomp_expected <NA>        NA
      281 Fleet               1 <NA>        agecomp_expected <NA>        NA
      282 Fleet               1 <NA>        agecomp_expected <NA>        NA
      283 Fleet               1 <NA>        agecomp_expected <NA>        NA
      284 Fleet               1 <NA>        agecomp_expected <NA>        NA
      285 Fleet               1 <NA>        agecomp_expected <NA>        NA
      286 Fleet               1 <NA>        agecomp_expected <NA>        NA
      287 Fleet               1 <NA>        agecomp_expected <NA>        NA
      288 Fleet               1 <NA>        agecomp_expected <NA>        NA
      289 Fleet               1 <NA>        agecomp_expected <NA>        NA
      290 Fleet               1 <NA>        agecomp_expected <NA>        NA
      291 Fleet               1 <NA>        agecomp_expected <NA>        NA
      292 Fleet               1 <NA>        agecomp_expected <NA>        NA
      293 Fleet               1 <NA>        agecomp_expected <NA>        NA
      294 Fleet               1 <NA>        agecomp_expected <NA>        NA
      295 Fleet               1 <NA>        agecomp_expected <NA>        NA
      296 Fleet               1 <NA>        agecomp_expected <NA>        NA
      297 Fleet               1 <NA>        agecomp_expected <NA>        NA
      298 Fleet               1 <NA>        agecomp_expected <NA>        NA
      299 Fleet               1 <NA>        agecomp_expected <NA>        NA
      300 Fleet               1 <NA>        agecomp_expected <NA>        NA
      301 Fleet               1 <NA>        agecomp_expected <NA>        NA
      302 Fleet               1 <NA>        agecomp_expected <NA>        NA
      303 Fleet               1 <NA>        agecomp_expected <NA>        NA
      304 Fleet               1 <NA>        agecomp_expected <NA>        NA
      305 Fleet               1 <NA>        agecomp_expected <NA>        NA
      306 Fleet               1 <NA>        agecomp_expected <NA>        NA
      307 Fleet               1 <NA>        agecomp_expected <NA>        NA
      308 Fleet               1 <NA>        agecomp_expected <NA>        NA
      309 Fleet               1 <NA>        agecomp_expected <NA>        NA
      310 Fleet               1 <NA>        agecomp_expected <NA>        NA
      311 Fleet               1 <NA>        agecomp_expected <NA>        NA
      312 Fleet               1 <NA>        agecomp_expected <NA>        NA
      313 Fleet               1 <NA>        agecomp_expected <NA>        NA
      314 Fleet               1 <NA>        agecomp_expected <NA>        NA
      315 Fleet               1 <NA>        agecomp_expected <NA>        NA
      316 Fleet               1 <NA>        agecomp_expected <NA>        NA
      317 Fleet               1 <NA>        agecomp_expected <NA>        NA
      318 Fleet               1 <NA>        agecomp_expected <NA>        NA
      319 Fleet               1 <NA>        agecomp_expected <NA>        NA
      320 Fleet               1 <NA>        agecomp_expected <NA>        NA
          parameter_id fleet year_i age_i stratum_i length_i  input observed
                 <int> <chr>  <int> <int>     <int>    <int>  <dbl>    <dbl>
        1            4 NA         1    NA        NA       NA -4.66        NA
        2          287 NA         2    NA        NA       NA -3.60        NA
        3          288 NA         3    NA        NA       NA -3.10        NA
        4          289 NA         4    NA        NA       NA -2.80        NA
        5          290 NA         5    NA        NA       NA -3.02        NA
        6          291 NA         6    NA        NA       NA -2.44        NA
        7          292 NA         7    NA        NA       NA -2.43        NA
        8          293 NA         8    NA        NA       NA -1.68        NA
        9          294 NA         9    NA        NA       NA -2.22        NA
       10          295 NA        10    NA        NA       NA -2.02        NA
       11          296 NA        11    NA        NA       NA -1.89        NA
       12          297 NA        12    NA        NA       NA -1.82        NA
       13          298 NA        13    NA        NA       NA -2.15        NA
       14          299 NA        14    NA        NA       NA -1.78        NA
       15          300 NA        15    NA        NA       NA -1.71        NA
       16          301 NA        16    NA        NA       NA -1.82        NA
       17          302 NA        17    NA        NA       NA -1.16        NA
       18          303 NA        18    NA        NA       NA -1.36        NA
       19          304 NA        19    NA        NA       NA -1.37        NA
       20          305 NA        20    NA        NA       NA -1.38        NA
       21          306 NA        21    NA        NA       NA -1.05        NA
       22          307 NA        22    NA        NA       NA -1.37        NA
       23          308 NA        23    NA        NA       NA -0.871       NA
       24          309 NA        24    NA        NA       NA -1.06        NA
       25          310 NA        25    NA        NA       NA -1.07        NA
       26          311 NA        26    NA        NA       NA -1.16        NA
       27          312 NA        27    NA        NA       NA -1.18        NA
       28          313 NA        28    NA        NA       NA -0.840       NA
       29          314 NA        29    NA        NA       NA -1.11        NA
       30          315 NA        30    NA        NA       NA -0.694       NA
       31            3 NA        NA    NA        NA       NA  0           NA
       32           NA NA         1     1        NA       NA NA           14
       33           NA NA         1     2        NA       NA NA           20
       34           NA NA         1     3        NA       NA NA           23
       35           NA NA         1     4        NA       NA NA           30
       36           NA NA         1     5        NA       NA NA           20
       37           NA NA         1     6        NA       NA NA           10
       38           NA NA         1     7        NA       NA NA           22
       39           NA NA         1     8        NA       NA NA           11
       40           NA NA         1     9        NA       NA NA           12
       41           NA NA         1    10        NA       NA NA            6
       42           NA NA         1    11        NA       NA NA            7
       43           NA NA         1    12        NA       NA NA           25
       44           NA NA         2     1        NA       NA NA           21
       45           NA NA         2     2        NA       NA NA           19
       46           NA NA         2     3        NA       NA NA           22
       47           NA NA         2     4        NA       NA NA           18
       48           NA NA         2     5        NA       NA NA           31
       49           NA NA         2     6        NA       NA NA           11
       50           NA NA         2     7        NA       NA NA           13
       51           NA NA         2     8        NA       NA NA           17
       52           NA NA         2     9        NA       NA NA            9
       53           NA NA         2    10        NA       NA NA            7
       54           NA NA         2    11        NA       NA NA            5
       55           NA NA         2    12        NA       NA NA           27
       56           NA NA         3     1        NA       NA NA           11
       57           NA NA         3     2        NA       NA NA           30
       58           NA NA         3     3        NA       NA NA           21
       59           NA NA         3     4        NA       NA NA           23
       60           NA NA         3     5        NA       NA NA           19
       61           NA NA         3     6        NA       NA NA           18
       62           NA NA         3     7        NA       NA NA           14
       63           NA NA         3     8        NA       NA NA           17
       64           NA NA         3     9        NA       NA NA            6
       65           NA NA         3    10        NA       NA NA            9
       66           NA NA         3    11        NA       NA NA            6
       67           NA NA         3    12        NA       NA NA           26
       68           NA NA         4     1        NA       NA NA           10
       69           NA NA         4     2        NA       NA NA           11
       70           NA NA         4     3        NA       NA NA           44
       71           NA NA         4     4        NA       NA NA           20
       72           NA NA         4     5        NA       NA NA           20
       73           NA NA         4     6        NA       NA NA           21
       74           NA NA         4     7        NA       NA NA           13
       75           NA NA         4     8        NA       NA NA           11
       76           NA NA         4     9        NA       NA NA           13
       77           NA NA         4    10        NA       NA NA            6
       78           NA NA         4    11        NA       NA NA            3
       79           NA NA         4    12        NA       NA NA           28
       80           NA NA         5     1        NA       NA NA           31
       81           NA NA         5     2        NA       NA NA           10
       82           NA NA         5     3        NA       NA NA           32
       83           NA NA         5     4        NA       NA NA           34
       84           NA NA         5     5        NA       NA NA           18
       85           NA NA         5     6        NA       NA NA           19
       86           NA NA         5     7        NA       NA NA            9
       87           NA NA         5     8        NA       NA NA            9
       88           NA NA         5     9        NA       NA NA            6
       89           NA NA         5    10        NA       NA NA            4
       90           NA NA         5    11        NA       NA NA            6
       91           NA NA         5    12        NA       NA NA           22
       92           NA NA         6     1        NA       NA NA           16
       93           NA NA         6     2        NA       NA NA           37
       94           NA NA         6     3        NA       NA NA           14
       95           NA NA         6     4        NA       NA NA           18
       96           NA NA         6     5        NA       NA NA           29
       97           NA NA         6     6        NA       NA NA           15
       98           NA NA         6     7        NA       NA NA           12
       99           NA NA         6     8        NA       NA NA           13
      100           NA NA         6     9        NA       NA NA            8
      101           NA NA         6    10        NA       NA NA            9
      102           NA NA         6    11        NA       NA NA            5
      103           NA NA         6    12        NA       NA NA           24
      104           NA NA         7     1        NA       NA NA           11
      105           NA NA         7     2        NA       NA NA           26
      106           NA NA         7     3        NA       NA NA           44
      107           NA NA         7     4        NA       NA NA           17
      108           NA NA         7     5        NA       NA NA           19
      109           NA NA         7     6        NA       NA NA           28
      110           NA NA         7     7        NA       NA NA            7
      111           NA NA         7     8        NA       NA NA            8
      112           NA NA         7     9        NA       NA NA            7
      113           NA NA         7    10        NA       NA NA            4
      114           NA NA         7    11        NA       NA NA            5
      115           NA NA         7    12        NA       NA NA           24
      116           NA NA         8     1        NA       NA NA           18
      117           NA NA         8     2        NA       NA NA           15
      118           NA NA         8     3        NA       NA NA           44
      119           NA NA         8     4        NA       NA NA           40
      120           NA NA         8     5        NA       NA NA           12
      121           NA NA         8     6        NA       NA NA            8
      122           NA NA         8     7        NA       NA NA           14
      123           NA NA         8     8        NA       NA NA           13
      124           NA NA         8     9        NA       NA NA            5
      125           NA NA         8    10        NA       NA NA            5
      126           NA NA         8    11        NA       NA NA            5
      127           NA NA         8    12        NA       NA NA           21
      128           NA NA         9     1        NA       NA NA           14
      129           NA NA         9     2        NA       NA NA           32
      130           NA NA         9     3        NA       NA NA           27
      131           NA NA         9     4        NA       NA NA           35
      132           NA NA         9     5        NA       NA NA           31
      133           NA NA         9     6        NA       NA NA            9
      134           NA NA         9     7        NA       NA NA            9
      135           NA NA         9     8        NA       NA NA           12
      136           NA NA         9     9        NA       NA NA            5
      137           NA NA         9    10        NA       NA NA            5
      138           NA NA         9    11        NA       NA NA            3
      139           NA NA         9    12        NA       NA NA           18
      140           NA NA        10     1        NA       NA NA           15
      141           NA NA        10     2        NA       NA NA           21
      142           NA NA        10     3        NA       NA NA           43
      143           NA NA        10     4        NA       NA NA           20
      144           NA NA        10     5        NA       NA NA           22
      145           NA NA        10     6        NA       NA NA           21
      146           NA NA        10     7        NA       NA NA            7
      147           NA NA        10     8        NA       NA NA            8
      148           NA NA        10     9        NA       NA NA           13
      149           NA NA        10    10        NA       NA NA            4
      150           NA NA        10    11        NA       NA NA            4
      151           NA NA        10    12        NA       NA NA           22
      152           NA NA        11     1        NA       NA NA           15
      153           NA NA        11     2        NA       NA NA           34
      154           NA NA        11     3        NA       NA NA           23
      155           NA NA        11     4        NA       NA NA           43
      156           NA NA        11     5        NA       NA NA           15
      157           NA NA        11     6        NA       NA NA           23
      158           NA NA        11     7        NA       NA NA           21
      159           NA NA        11     8        NA       NA NA            5
      160           NA NA        11     9        NA       NA NA            3
      161           NA NA        11    10        NA       NA NA            5
      162           NA NA        11    11        NA       NA NA            2
      163           NA NA        11    12        NA       NA NA           11
      164           NA NA        12     1        NA       NA NA           11
      165           NA NA        12     2        NA       NA NA           32
      166           NA NA        12     3        NA       NA NA           30
      167           NA NA        12     4        NA       NA NA           26
      168           NA NA        12     5        NA       NA NA           33
      169           NA NA        12     6        NA       NA NA           14
      170           NA NA        12     7        NA       NA NA           13
      171           NA NA        12     8        NA       NA NA           21
      172           NA NA        12     9        NA       NA NA            4
      173           NA NA        12    10        NA       NA NA            3
      174           NA NA        12    11        NA       NA NA            4
      175           NA NA        12    12        NA       NA NA            9
      176           NA NA        13     1        NA       NA NA           22
      177           NA NA        13     2        NA       NA NA           21
      178           NA NA        13     3        NA       NA NA           23
      179           NA NA        13     4        NA       NA NA           28
      180           NA NA        13     5        NA       NA NA           23
      181           NA NA        13     6        NA       NA NA           18
      182           NA NA        13     7        NA       NA NA           15
      183           NA NA        13     8        NA       NA NA           18
      184           NA NA        13     9        NA       NA NA           16
      185           NA NA        13    10        NA       NA NA            3
      186           NA NA        13    11        NA       NA NA            3
      187           NA NA        13    12        NA       NA NA           10
      188           NA NA        14     1        NA       NA NA           23
      189           NA NA        14     2        NA       NA NA           28
      190           NA NA        14     3        NA       NA NA           17
      191           NA NA        14     4        NA       NA NA           41
      192           NA NA        14     5        NA       NA NA           24
      193           NA NA        14     6        NA       NA NA           10
      194           NA NA        14     7        NA       NA NA           10
      195           NA NA        14     8        NA       NA NA            8
      196           NA NA        14     9        NA       NA NA            9
      197           NA NA        14    10        NA       NA NA           12
      198           NA NA        14    11        NA       NA NA            5
      199           NA NA        14    12        NA       NA NA           13
      200           NA NA        15     1        NA       NA NA           14
      201           NA NA        15     2        NA       NA NA           38
      202           NA NA        15     3        NA       NA NA           39
      203           NA NA        15     4        NA       NA NA           16
      204           NA NA        15     5        NA       NA NA           28
      205           NA NA        15     6        NA       NA NA           21
      206           NA NA        15     7        NA       NA NA           10
      207           NA NA        15     8        NA       NA NA           11
      208           NA NA        15     9        NA       NA NA            5
      209           NA NA        15    10        NA       NA NA            4
      210           NA NA        15    11        NA       NA NA            0
      211           NA NA        15    12        NA       NA NA           14
      212           NA NA        16     1        NA       NA NA           29
      213           NA NA        16     2        NA       NA NA           25
      214           NA NA        16     3        NA       NA NA           39
      215           NA NA        16     4        NA       NA NA           25
      216           NA NA        16     5        NA       NA NA           17
      217           NA NA        16     6        NA       NA NA           13
      218           NA NA        16     7        NA       NA NA           12
      219           NA NA        16     8        NA       NA NA            8
      220           NA NA        16     9        NA       NA NA            7
      221           NA NA        16    10        NA       NA NA            3
      222           NA NA        16    11        NA       NA NA            7
      223           NA NA        16    12        NA       NA NA           15
      224           NA NA        17     1        NA       NA NA           33
      225           NA NA        17     2        NA       NA NA           38
      226           NA NA        17     3        NA       NA NA           28
      227           NA NA        17     4        NA       NA NA           29
      228           NA NA        17     5        NA       NA NA           24
      229           NA NA        17     6        NA       NA NA            6
      230           NA NA        17     7        NA       NA NA           10
      231           NA NA        17     8        NA       NA NA            1
      232           NA NA        17     9        NA       NA NA            6
      233           NA NA        17    10        NA       NA NA            5
      234           NA NA        17    11        NA       NA NA            2
      235           NA NA        17    12        NA       NA NA           18
      236           NA NA        18     1        NA       NA NA           17
      237           NA NA        18     2        NA       NA NA           40
      238           NA NA        18     3        NA       NA NA           50
      239           NA NA        18     4        NA       NA NA           25
      240           NA NA        18     5        NA       NA NA           17
      241           NA NA        18     6        NA       NA NA           13
      242           NA NA        18     7        NA       NA NA           11
      243           NA NA        18     8        NA       NA NA            8
      244           NA NA        18     9        NA       NA NA            7
      245           NA NA        18    10        NA       NA NA            1
      246           NA NA        18    11        NA       NA NA            4
      247           NA NA        18    12        NA       NA NA            7
      248           NA NA        19     1        NA       NA NA           26
      249           NA NA        19     2        NA       NA NA           24
      250           NA NA        19     3        NA       NA NA           37
      251           NA NA        19     4        NA       NA NA           34
      252           NA NA        19     5        NA       NA NA           22
      253           NA NA        19     6        NA       NA NA           23
      254           NA NA        19     7        NA       NA NA            8
      255           NA NA        19     8        NA       NA NA            6
      256           NA NA        19     9        NA       NA NA            3
      257           NA NA        19    10        NA       NA NA            5
      258           NA NA        19    11        NA       NA NA            4
      259           NA NA        19    12        NA       NA NA            8
      260           NA NA        20     1        NA       NA NA            9
      261           NA NA        20     2        NA       NA NA           31
      262           NA NA        20     3        NA       NA NA           33
      263           NA NA        20     4        NA       NA NA           51
      264           NA NA        20     5        NA       NA NA           29
      265           NA NA        20     6        NA       NA NA           12
      266           NA NA        20     7        NA       NA NA           11
      267           NA NA        20     8        NA       NA NA           10
      268           NA NA        20     9        NA       NA NA            5
      269           NA NA        20    10        NA       NA NA            2
      270           NA NA        20    11        NA       NA NA            5
      271           NA NA        20    12        NA       NA NA            2
      272           NA NA        21     1        NA       NA NA           49
      273           NA NA        21     2        NA       NA NA           15
      274           NA NA        21     3        NA       NA NA           38
      275           NA NA        21     4        NA       NA NA           23
      276           NA NA        21     5        NA       NA NA           25
      277           NA NA        21     6        NA       NA NA           17
      278           NA NA        21     7        NA       NA NA           10
      279           NA NA        21     8        NA       NA NA            8
      280           NA NA        21     9        NA       NA NA            2
      281           NA NA        21    10        NA       NA NA            1
      282           NA NA        21    11        NA       NA NA            5
      283           NA NA        21    12        NA       NA NA            7
      284           NA NA        22     1        NA       NA NA           40
      285           NA NA        22     2        NA       NA NA           67
      286           NA NA        22     3        NA       NA NA           14
      287           NA NA        22     4        NA       NA NA           23
      288           NA NA        22     5        NA       NA NA           15
      289           NA NA        22     6        NA       NA NA            6
      290           NA NA        22     7        NA       NA NA           14
      291           NA NA        22     8        NA       NA NA            4
      292           NA NA        22     9        NA       NA NA            5
      293           NA NA        22    10        NA       NA NA            4
      294           NA NA        22    11        NA       NA NA            1
      295           NA NA        22    12        NA       NA NA            7
      296           NA NA        23     1        NA       NA NA           22
      297           NA NA        23     2        NA       NA NA           51
      298           NA NA        23     3        NA       NA NA           56
      299           NA NA        23     4        NA       NA NA           16
      300           NA NA        23     5        NA       NA NA           11
      301           NA NA        23     6        NA       NA NA            7
      302           NA NA        23     7        NA       NA NA           18
      303           NA NA        23     8        NA       NA NA            5
      304           NA NA        23     9        NA       NA NA            1
      305           NA NA        23    10        NA       NA NA            3
      306           NA NA        23    11        NA       NA NA            5
      307           NA NA        23    12        NA       NA NA            5
      308           NA NA        24     1        NA       NA NA           24
      309           NA NA        24     2        NA       NA NA           38
      310           NA NA        24     3        NA       NA NA           48
      311           NA NA        24     4        NA       NA NA           36
      312           NA NA        24     5        NA       NA NA           14
      313           NA NA        24     6        NA       NA NA           15
      314           NA NA        24     7        NA       NA NA            4
      315           NA NA        24     8        NA       NA NA            6
      316           NA NA        24     9        NA       NA NA            8
      317           NA NA        24    10        NA       NA NA            1
      318           NA NA        24    11        NA       NA NA            4
      319           NA NA        24    12        NA       NA NA            2
      320           NA NA        25     1        NA       NA NA           36
          estimation_type  distribution input_type  lpdf log_sd
          <chr>            <chr>        <chr>      <dbl>  <dbl>
        1 fixed_effects    <NA>         <NA>         NA      NA
        2 fixed_effects    <NA>         <NA>         NA      NA
        3 fixed_effects    <NA>         <NA>         NA      NA
        4 fixed_effects    <NA>         <NA>         NA      NA
        5 fixed_effects    <NA>         <NA>         NA      NA
        6 fixed_effects    <NA>         <NA>         NA      NA
        7 fixed_effects    <NA>         <NA>         NA      NA
        8 fixed_effects    <NA>         <NA>         NA      NA
        9 fixed_effects    <NA>         <NA>         NA      NA
       10 fixed_effects    <NA>         <NA>         NA      NA
       11 fixed_effects    <NA>         <NA>         NA      NA
       12 fixed_effects    <NA>         <NA>         NA      NA
       13 fixed_effects    <NA>         <NA>         NA      NA
       14 fixed_effects    <NA>         <NA>         NA      NA
       15 fixed_effects    <NA>         <NA>         NA      NA
       16 fixed_effects    <NA>         <NA>         NA      NA
       17 fixed_effects    <NA>         <NA>         NA      NA
       18 fixed_effects    <NA>         <NA>         NA      NA
       19 fixed_effects    <NA>         <NA>         NA      NA
       20 fixed_effects    <NA>         <NA>         NA      NA
       21 fixed_effects    <NA>         <NA>         NA      NA
       22 fixed_effects    <NA>         <NA>         NA      NA
       23 fixed_effects    <NA>         <NA>         NA      NA
       24 fixed_effects    <NA>         <NA>         NA      NA
       25 fixed_effects    <NA>         <NA>         NA      NA
       26 fixed_effects    <NA>         <NA>         NA      NA
       27 fixed_effects    <NA>         <NA>         NA      NA
       28 fixed_effects    <NA>         <NA>         NA      NA
       29 fixed_effects    <NA>         <NA>         NA      NA
       30 fixed_effects    <NA>         <NA>         NA      NA
       31 constant         <NA>         <NA>         NA      NA
       32 derived_quantity multinomial  data       -836.     NA
       33 derived_quantity multinomial  data       -836.     NA
       34 derived_quantity multinomial  data       -836.     NA
       35 derived_quantity multinomial  data       -836.     NA
       36 derived_quantity multinomial  data       -836.     NA
       37 derived_quantity multinomial  data       -836.     NA
       38 derived_quantity multinomial  data       -836.     NA
       39 derived_quantity multinomial  data       -836.     NA
       40 derived_quantity multinomial  data       -836.     NA
       41 derived_quantity multinomial  data       -836.     NA
       42 derived_quantity multinomial  data       -836.     NA
       43 derived_quantity multinomial  data       -836.     NA
       44 derived_quantity multinomial  data       -836.     NA
       45 derived_quantity multinomial  data       -836.     NA
       46 derived_quantity multinomial  data       -836.     NA
       47 derived_quantity multinomial  data       -836.     NA
       48 derived_quantity multinomial  data       -836.     NA
       49 derived_quantity multinomial  data       -836.     NA
       50 derived_quantity multinomial  data       -836.     NA
       51 derived_quantity multinomial  data       -836.     NA
       52 derived_quantity multinomial  data       -836.     NA
       53 derived_quantity multinomial  data       -836.     NA
       54 derived_quantity multinomial  data       -836.     NA
       55 derived_quantity multinomial  data       -836.     NA
       56 derived_quantity multinomial  data       -836.     NA
       57 derived_quantity multinomial  data       -836.     NA
       58 derived_quantity multinomial  data       -836.     NA
       59 derived_quantity multinomial  data       -836.     NA
       60 derived_quantity multinomial  data       -836.     NA
       61 derived_quantity multinomial  data       -836.     NA
       62 derived_quantity multinomial  data       -836.     NA
       63 derived_quantity multinomial  data       -836.     NA
       64 derived_quantity multinomial  data       -836.     NA
       65 derived_quantity multinomial  data       -836.     NA
       66 derived_quantity multinomial  data       -836.     NA
       67 derived_quantity multinomial  data       -836.     NA
       68 derived_quantity multinomial  data       -836.     NA
       69 derived_quantity multinomial  data       -836.     NA
       70 derived_quantity multinomial  data       -836.     NA
       71 derived_quantity multinomial  data       -836.     NA
       72 derived_quantity multinomial  data       -836.     NA
       73 derived_quantity multinomial  data       -836.     NA
       74 derived_quantity multinomial  data       -836.     NA
       75 derived_quantity multinomial  data       -836.     NA
       76 derived_quantity multinomial  data       -836.     NA
       77 derived_quantity multinomial  data       -836.     NA
       78 derived_quantity multinomial  data       -836.     NA
       79 derived_quantity multinomial  data       -836.     NA
       80 derived_quantity multinomial  data       -836.     NA
       81 derived_quantity multinomial  data       -836.     NA
       82 derived_quantity multinomial  data       -836.     NA
       83 derived_quantity multinomial  data       -836.     NA
       84 derived_quantity multinomial  data       -836.     NA
       85 derived_quantity multinomial  data       -836.     NA
       86 derived_quantity multinomial  data       -836.     NA
       87 derived_quantity multinomial  data       -836.     NA
       88 derived_quantity multinomial  data       -836.     NA
       89 derived_quantity multinomial  data       -836.     NA
       90 derived_quantity multinomial  data       -836.     NA
       91 derived_quantity multinomial  data       -836.     NA
       92 derived_quantity multinomial  data       -836.     NA
       93 derived_quantity multinomial  data       -836.     NA
       94 derived_quantity multinomial  data       -836.     NA
       95 derived_quantity multinomial  data       -836.     NA
       96 derived_quantity multinomial  data       -836.     NA
       97 derived_quantity multinomial  data       -836.     NA
       98 derived_quantity multinomial  data       -836.     NA
       99 derived_quantity multinomial  data       -836.     NA
      100 derived_quantity multinomial  data       -836.     NA
      101 derived_quantity multinomial  data       -836.     NA
      102 derived_quantity multinomial  data       -836.     NA
      103 derived_quantity multinomial  data       -836.     NA
      104 derived_quantity multinomial  data       -836.     NA
      105 derived_quantity multinomial  data       -836.     NA
      106 derived_quantity multinomial  data       -836.     NA
      107 derived_quantity multinomial  data       -836.     NA
      108 derived_quantity multinomial  data       -836.     NA
      109 derived_quantity multinomial  data       -836.     NA
      110 derived_quantity multinomial  data       -836.     NA
      111 derived_quantity multinomial  data       -836.     NA
      112 derived_quantity multinomial  data       -836.     NA
      113 derived_quantity multinomial  data       -836.     NA
      114 derived_quantity multinomial  data       -836.     NA
      115 derived_quantity multinomial  data       -836.     NA
      116 derived_quantity multinomial  data       -836.     NA
      117 derived_quantity multinomial  data       -836.     NA
      118 derived_quantity multinomial  data       -836.     NA
      119 derived_quantity multinomial  data       -836.     NA
      120 derived_quantity multinomial  data       -836.     NA
      121 derived_quantity multinomial  data       -836.     NA
      122 derived_quantity multinomial  data       -836.     NA
      123 derived_quantity multinomial  data       -836.     NA
      124 derived_quantity multinomial  data       -836.     NA
      125 derived_quantity multinomial  data       -836.     NA
      126 derived_quantity multinomial  data       -836.     NA
      127 derived_quantity multinomial  data       -836.     NA
      128 derived_quantity multinomial  data       -836.     NA
      129 derived_quantity multinomial  data       -836.     NA
      130 derived_quantity multinomial  data       -836.     NA
      131 derived_quantity multinomial  data       -836.     NA
      132 derived_quantity multinomial  data       -836.     NA
      133 derived_quantity multinomial  data       -836.     NA
      134 derived_quantity multinomial  data       -836.     NA
      135 derived_quantity multinomial  data       -836.     NA
      136 derived_quantity multinomial  data       -836.     NA
      137 derived_quantity multinomial  data       -836.     NA
      138 derived_quantity multinomial  data       -836.     NA
      139 derived_quantity multinomial  data       -836.     NA
      140 derived_quantity multinomial  data       -836.     NA
      141 derived_quantity multinomial  data       -836.     NA
      142 derived_quantity multinomial  data       -836.     NA
      143 derived_quantity multinomial  data       -836.     NA
      144 derived_quantity multinomial  data       -836.     NA
      145 derived_quantity multinomial  data       -836.     NA
      146 derived_quantity multinomial  data       -836.     NA
      147 derived_quantity multinomial  data       -836.     NA
      148 derived_quantity multinomial  data       -836.     NA
      149 derived_quantity multinomial  data       -836.     NA
      150 derived_quantity multinomial  data       -836.     NA
      151 derived_quantity multinomial  data       -836.     NA
      152 derived_quantity multinomial  data       -836.     NA
      153 derived_quantity multinomial  data       -836.     NA
      154 derived_quantity multinomial  data       -836.     NA
      155 derived_quantity multinomial  data       -836.     NA
      156 derived_quantity multinomial  data       -836.     NA
      157 derived_quantity multinomial  data       -836.     NA
      158 derived_quantity multinomial  data       -836.     NA
      159 derived_quantity multinomial  data       -836.     NA
      160 derived_quantity multinomial  data       -836.     NA
      161 derived_quantity multinomial  data       -836.     NA
      162 derived_quantity multinomial  data       -836.     NA
      163 derived_quantity multinomial  data       -836.     NA
      164 derived_quantity multinomial  data       -836.     NA
      165 derived_quantity multinomial  data       -836.     NA
      166 derived_quantity multinomial  data       -836.     NA
      167 derived_quantity multinomial  data       -836.     NA
      168 derived_quantity multinomial  data       -836.     NA
      169 derived_quantity multinomial  data       -836.     NA
      170 derived_quantity multinomial  data       -836.     NA
      171 derived_quantity multinomial  data       -836.     NA
      172 derived_quantity multinomial  data       -836.     NA
      173 derived_quantity multinomial  data       -836.     NA
      174 derived_quantity multinomial  data       -836.     NA
      175 derived_quantity multinomial  data       -836.     NA
      176 derived_quantity multinomial  data       -836.     NA
      177 derived_quantity multinomial  data       -836.     NA
      178 derived_quantity multinomial  data       -836.     NA
      179 derived_quantity multinomial  data       -836.     NA
      180 derived_quantity multinomial  data       -836.     NA
      181 derived_quantity multinomial  data       -836.     NA
      182 derived_quantity multinomial  data       -836.     NA
      183 derived_quantity multinomial  data       -836.     NA
      184 derived_quantity multinomial  data       -836.     NA
      185 derived_quantity multinomial  data       -836.     NA
      186 derived_quantity multinomial  data       -836.     NA
      187 derived_quantity multinomial  data       -836.     NA
      188 derived_quantity multinomial  data       -836.     NA
      189 derived_quantity multinomial  data       -836.     NA
      190 derived_quantity multinomial  data       -836.     NA
      191 derived_quantity multinomial  data       -836.     NA
      192 derived_quantity multinomial  data       -836.     NA
      193 derived_quantity multinomial  data       -836.     NA
      194 derived_quantity multinomial  data       -836.     NA
      195 derived_quantity multinomial  data       -836.     NA
      196 derived_quantity multinomial  data       -836.     NA
      197 derived_quantity multinomial  data       -836.     NA
      198 derived_quantity multinomial  data       -836.     NA
      199 derived_quantity multinomial  data       -836.     NA
      200 derived_quantity multinomial  data       -836.     NA
      201 derived_quantity multinomial  data       -836.     NA
      202 derived_quantity multinomial  data       -836.     NA
      203 derived_quantity multinomial  data       -836.     NA
      204 derived_quantity multinomial  data       -836.     NA
      205 derived_quantity multinomial  data       -836.     NA
      206 derived_quantity multinomial  data       -836.     NA
      207 derived_quantity multinomial  data       -836.     NA
      208 derived_quantity multinomial  data       -836.     NA
      209 derived_quantity multinomial  data       -836.     NA
      210 derived_quantity multinomial  data       -836.     NA
      211 derived_quantity multinomial  data       -836.     NA
      212 derived_quantity multinomial  data       -836.     NA
      213 derived_quantity multinomial  data       -836.     NA
      214 derived_quantity multinomial  data       -836.     NA
      215 derived_quantity multinomial  data       -836.     NA
      216 derived_quantity multinomial  data       -836.     NA
      217 derived_quantity multinomial  data       -836.     NA
      218 derived_quantity multinomial  data       -836.     NA
      219 derived_quantity multinomial  data       -836.     NA
      220 derived_quantity multinomial  data       -836.     NA
      221 derived_quantity multinomial  data       -836.     NA
      222 derived_quantity multinomial  data       -836.     NA
      223 derived_quantity multinomial  data       -836.     NA
      224 derived_quantity multinomial  data       -836.     NA
      225 derived_quantity multinomial  data       -836.     NA
      226 derived_quantity multinomial  data       -836.     NA
      227 derived_quantity multinomial  data       -836.     NA
      228 derived_quantity multinomial  data       -836.     NA
      229 derived_quantity multinomial  data       -836.     NA
      230 derived_quantity multinomial  data       -836.     NA
      231 derived_quantity multinomial  data       -836.     NA
      232 derived_quantity multinomial  data       -836.     NA
      233 derived_quantity multinomial  data       -836.     NA
      234 derived_quantity multinomial  data       -836.     NA
      235 derived_quantity multinomial  data       -836.     NA
      236 derived_quantity multinomial  data       -836.     NA
      237 derived_quantity multinomial  data       -836.     NA
      238 derived_quantity multinomial  data       -836.     NA
      239 derived_quantity multinomial  data       -836.     NA
      240 derived_quantity multinomial  data       -836.     NA
      241 derived_quantity multinomial  data       -836.     NA
      242 derived_quantity multinomial  data       -836.     NA
      243 derived_quantity multinomial  data       -836.     NA
      244 derived_quantity multinomial  data       -836.     NA
      245 derived_quantity multinomial  data       -836.     NA
      246 derived_quantity multinomial  data       -836.     NA
      247 derived_quantity multinomial  data       -836.     NA
      248 derived_quantity multinomial  data       -836.     NA
      249 derived_quantity multinomial  data       -836.     NA
      250 derived_quantity multinomial  data       -836.     NA
      251 derived_quantity multinomial  data       -836.     NA
      252 derived_quantity multinomial  data       -836.     NA
      253 derived_quantity multinomial  data       -836.     NA
      254 derived_quantity multinomial  data       -836.     NA
      255 derived_quantity multinomial  data       -836.     NA
      256 derived_quantity multinomial  data       -836.     NA
      257 derived_quantity multinomial  data       -836.     NA
      258 derived_quantity multinomial  data       -836.     NA
      259 derived_quantity multinomial  data       -836.     NA
      260 derived_quantity multinomial  data       -836.     NA
      261 derived_quantity multinomial  data       -836.     NA
      262 derived_quantity multinomial  data       -836.     NA
      263 derived_quantity multinomial  data       -836.     NA
      264 derived_quantity multinomial  data       -836.     NA
      265 derived_quantity multinomial  data       -836.     NA
      266 derived_quantity multinomial  data       -836.     NA
      267 derived_quantity multinomial  data       -836.     NA
      268 derived_quantity multinomial  data       -836.     NA
      269 derived_quantity multinomial  data       -836.     NA
      270 derived_quantity multinomial  data       -836.     NA
      271 derived_quantity multinomial  data       -836.     NA
      272 derived_quantity multinomial  data       -836.     NA
      273 derived_quantity multinomial  data       -836.     NA
      274 derived_quantity multinomial  data       -836.     NA
      275 derived_quantity multinomial  data       -836.     NA
      276 derived_quantity multinomial  data       -836.     NA
      277 derived_quantity multinomial  data       -836.     NA
      278 derived_quantity multinomial  data       -836.     NA
      279 derived_quantity multinomial  data       -836.     NA
      280 derived_quantity multinomial  data       -836.     NA
      281 derived_quantity multinomial  data       -836.     NA
      282 derived_quantity multinomial  data       -836.     NA
      283 derived_quantity multinomial  data       -836.     NA
      284 derived_quantity multinomial  data       -836.     NA
      285 derived_quantity multinomial  data       -836.     NA
      286 derived_quantity multinomial  data       -836.     NA
      287 derived_quantity multinomial  data       -836.     NA
      288 derived_quantity multinomial  data       -836.     NA
      289 derived_quantity multinomial  data       -836.     NA
      290 derived_quantity multinomial  data       -836.     NA
      291 derived_quantity multinomial  data       -836.     NA
      292 derived_quantity multinomial  data       -836.     NA
      293 derived_quantity multinomial  data       -836.     NA
      294 derived_quantity multinomial  data       -836.     NA
      295 derived_quantity multinomial  data       -836.     NA
      296 derived_quantity multinomial  data       -836.     NA
      297 derived_quantity multinomial  data       -836.     NA
      298 derived_quantity multinomial  data       -836.     NA
      299 derived_quantity multinomial  data       -836.     NA
      300 derived_quantity multinomial  data       -836.     NA
      301 derived_quantity multinomial  data       -836.     NA
      302 derived_quantity multinomial  data       -836.     NA
      303 derived_quantity multinomial  data       -836.     NA
      304 derived_quantity multinomial  data       -836.     NA
      305 derived_quantity multinomial  data       -836.     NA
      306 derived_quantity multinomial  data       -836.     NA
      307 derived_quantity multinomial  data       -836.     NA
      308 derived_quantity multinomial  data       -836.     NA
      309 derived_quantity multinomial  data       -836.     NA
      310 derived_quantity multinomial  data       -836.     NA
      311 derived_quantity multinomial  data       -836.     NA
      312 derived_quantity multinomial  data       -836.     NA
      313 derived_quantity multinomial  data       -836.     NA
      314 derived_quantity multinomial  data       -836.     NA
      315 derived_quantity multinomial  data       -836.     NA
      316 derived_quantity multinomial  data       -836.     NA
      317 derived_quantity multinomial  data       -836.     NA
      318 derived_quantity multinomial  data       -836.     NA
      319 derived_quantity multinomial  data       -836.     NA
      320 derived_quantity multinomial  data       -836.     NA
      # i 19,496 more rows

# `get_estimates()` works with estimation run

    Code
      print(dplyr::select(get_estimates(readRDS(fit_files[[1]])), -estimated,
      -expected, -uncertainty, -gradient, -likelihood, -log_like_cv, -gradient), n = 320,
      width = Inf)
    Output
      # A tibble: 19,816 x 19
          module_name module_id module_type label            type   type_id
          <chr>           <int> <chr>       <chr>            <chr>    <int>
        1 Fleet               1 <NA>        log_Fmort        vector       4
        2 Fleet               1 <NA>        log_Fmort        vector       4
        3 Fleet               1 <NA>        log_Fmort        vector       4
        4 Fleet               1 <NA>        log_Fmort        vector       4
        5 Fleet               1 <NA>        log_Fmort        vector       4
        6 Fleet               1 <NA>        log_Fmort        vector       4
        7 Fleet               1 <NA>        log_Fmort        vector       4
        8 Fleet               1 <NA>        log_Fmort        vector       4
        9 Fleet               1 <NA>        log_Fmort        vector       4
       10 Fleet               1 <NA>        log_Fmort        vector       4
       11 Fleet               1 <NA>        log_Fmort        vector       4
       12 Fleet               1 <NA>        log_Fmort        vector       4
       13 Fleet               1 <NA>        log_Fmort        vector       4
       14 Fleet               1 <NA>        log_Fmort        vector       4
       15 Fleet               1 <NA>        log_Fmort        vector       4
       16 Fleet               1 <NA>        log_Fmort        vector       4
       17 Fleet               1 <NA>        log_Fmort        vector       4
       18 Fleet               1 <NA>        log_Fmort        vector       4
       19 Fleet               1 <NA>        log_Fmort        vector       4
       20 Fleet               1 <NA>        log_Fmort        vector       4
       21 Fleet               1 <NA>        log_Fmort        vector       4
       22 Fleet               1 <NA>        log_Fmort        vector       4
       23 Fleet               1 <NA>        log_Fmort        vector       4
       24 Fleet               1 <NA>        log_Fmort        vector       4
       25 Fleet               1 <NA>        log_Fmort        vector       4
       26 Fleet               1 <NA>        log_Fmort        vector       4
       27 Fleet               1 <NA>        log_Fmort        vector       4
       28 Fleet               1 <NA>        log_Fmort        vector       4
       29 Fleet               1 <NA>        log_Fmort        vector       4
       30 Fleet               1 <NA>        log_Fmort        vector       4
       31 Fleet               1 <NA>        log_q            vector       3
       32 Fleet               1 <NA>        agecomp_expected <NA>        NA
       33 Fleet               1 <NA>        agecomp_expected <NA>        NA
       34 Fleet               1 <NA>        agecomp_expected <NA>        NA
       35 Fleet               1 <NA>        agecomp_expected <NA>        NA
       36 Fleet               1 <NA>        agecomp_expected <NA>        NA
       37 Fleet               1 <NA>        agecomp_expected <NA>        NA
       38 Fleet               1 <NA>        agecomp_expected <NA>        NA
       39 Fleet               1 <NA>        agecomp_expected <NA>        NA
       40 Fleet               1 <NA>        agecomp_expected <NA>        NA
       41 Fleet               1 <NA>        agecomp_expected <NA>        NA
       42 Fleet               1 <NA>        agecomp_expected <NA>        NA
       43 Fleet               1 <NA>        agecomp_expected <NA>        NA
       44 Fleet               1 <NA>        agecomp_expected <NA>        NA
       45 Fleet               1 <NA>        agecomp_expected <NA>        NA
       46 Fleet               1 <NA>        agecomp_expected <NA>        NA
       47 Fleet               1 <NA>        agecomp_expected <NA>        NA
       48 Fleet               1 <NA>        agecomp_expected <NA>        NA
       49 Fleet               1 <NA>        agecomp_expected <NA>        NA
       50 Fleet               1 <NA>        agecomp_expected <NA>        NA
       51 Fleet               1 <NA>        agecomp_expected <NA>        NA
       52 Fleet               1 <NA>        agecomp_expected <NA>        NA
       53 Fleet               1 <NA>        agecomp_expected <NA>        NA
       54 Fleet               1 <NA>        agecomp_expected <NA>        NA
       55 Fleet               1 <NA>        agecomp_expected <NA>        NA
       56 Fleet               1 <NA>        agecomp_expected <NA>        NA
       57 Fleet               1 <NA>        agecomp_expected <NA>        NA
       58 Fleet               1 <NA>        agecomp_expected <NA>        NA
       59 Fleet               1 <NA>        agecomp_expected <NA>        NA
       60 Fleet               1 <NA>        agecomp_expected <NA>        NA
       61 Fleet               1 <NA>        agecomp_expected <NA>        NA
       62 Fleet               1 <NA>        agecomp_expected <NA>        NA
       63 Fleet               1 <NA>        agecomp_expected <NA>        NA
       64 Fleet               1 <NA>        agecomp_expected <NA>        NA
       65 Fleet               1 <NA>        agecomp_expected <NA>        NA
       66 Fleet               1 <NA>        agecomp_expected <NA>        NA
       67 Fleet               1 <NA>        agecomp_expected <NA>        NA
       68 Fleet               1 <NA>        agecomp_expected <NA>        NA
       69 Fleet               1 <NA>        agecomp_expected <NA>        NA
       70 Fleet               1 <NA>        agecomp_expected <NA>        NA
       71 Fleet               1 <NA>        agecomp_expected <NA>        NA
       72 Fleet               1 <NA>        agecomp_expected <NA>        NA
       73 Fleet               1 <NA>        agecomp_expected <NA>        NA
       74 Fleet               1 <NA>        agecomp_expected <NA>        NA
       75 Fleet               1 <NA>        agecomp_expected <NA>        NA
       76 Fleet               1 <NA>        agecomp_expected <NA>        NA
       77 Fleet               1 <NA>        agecomp_expected <NA>        NA
       78 Fleet               1 <NA>        agecomp_expected <NA>        NA
       79 Fleet               1 <NA>        agecomp_expected <NA>        NA
       80 Fleet               1 <NA>        agecomp_expected <NA>        NA
       81 Fleet               1 <NA>        agecomp_expected <NA>        NA
       82 Fleet               1 <NA>        agecomp_expected <NA>        NA
       83 Fleet               1 <NA>        agecomp_expected <NA>        NA
       84 Fleet               1 <NA>        agecomp_expected <NA>        NA
       85 Fleet               1 <NA>        agecomp_expected <NA>        NA
       86 Fleet               1 <NA>        agecomp_expected <NA>        NA
       87 Fleet               1 <NA>        agecomp_expected <NA>        NA
       88 Fleet               1 <NA>        agecomp_expected <NA>        NA
       89 Fleet               1 <NA>        agecomp_expected <NA>        NA
       90 Fleet               1 <NA>        agecomp_expected <NA>        NA
       91 Fleet               1 <NA>        agecomp_expected <NA>        NA
       92 Fleet               1 <NA>        agecomp_expected <NA>        NA
       93 Fleet               1 <NA>        agecomp_expected <NA>        NA
       94 Fleet               1 <NA>        agecomp_expected <NA>        NA
       95 Fleet               1 <NA>        agecomp_expected <NA>        NA
       96 Fleet               1 <NA>        agecomp_expected <NA>        NA
       97 Fleet               1 <NA>        agecomp_expected <NA>        NA
       98 Fleet               1 <NA>        agecomp_expected <NA>        NA
       99 Fleet               1 <NA>        agecomp_expected <NA>        NA
      100 Fleet               1 <NA>        agecomp_expected <NA>        NA
      101 Fleet               1 <NA>        agecomp_expected <NA>        NA
      102 Fleet               1 <NA>        agecomp_expected <NA>        NA
      103 Fleet               1 <NA>        agecomp_expected <NA>        NA
      104 Fleet               1 <NA>        agecomp_expected <NA>        NA
      105 Fleet               1 <NA>        agecomp_expected <NA>        NA
      106 Fleet               1 <NA>        agecomp_expected <NA>        NA
      107 Fleet               1 <NA>        agecomp_expected <NA>        NA
      108 Fleet               1 <NA>        agecomp_expected <NA>        NA
      109 Fleet               1 <NA>        agecomp_expected <NA>        NA
      110 Fleet               1 <NA>        agecomp_expected <NA>        NA
      111 Fleet               1 <NA>        agecomp_expected <NA>        NA
      112 Fleet               1 <NA>        agecomp_expected <NA>        NA
      113 Fleet               1 <NA>        agecomp_expected <NA>        NA
      114 Fleet               1 <NA>        agecomp_expected <NA>        NA
      115 Fleet               1 <NA>        agecomp_expected <NA>        NA
      116 Fleet               1 <NA>        agecomp_expected <NA>        NA
      117 Fleet               1 <NA>        agecomp_expected <NA>        NA
      118 Fleet               1 <NA>        agecomp_expected <NA>        NA
      119 Fleet               1 <NA>        agecomp_expected <NA>        NA
      120 Fleet               1 <NA>        agecomp_expected <NA>        NA
      121 Fleet               1 <NA>        agecomp_expected <NA>        NA
      122 Fleet               1 <NA>        agecomp_expected <NA>        NA
      123 Fleet               1 <NA>        agecomp_expected <NA>        NA
      124 Fleet               1 <NA>        agecomp_expected <NA>        NA
      125 Fleet               1 <NA>        agecomp_expected <NA>        NA
      126 Fleet               1 <NA>        agecomp_expected <NA>        NA
      127 Fleet               1 <NA>        agecomp_expected <NA>        NA
      128 Fleet               1 <NA>        agecomp_expected <NA>        NA
      129 Fleet               1 <NA>        agecomp_expected <NA>        NA
      130 Fleet               1 <NA>        agecomp_expected <NA>        NA
      131 Fleet               1 <NA>        agecomp_expected <NA>        NA
      132 Fleet               1 <NA>        agecomp_expected <NA>        NA
      133 Fleet               1 <NA>        agecomp_expected <NA>        NA
      134 Fleet               1 <NA>        agecomp_expected <NA>        NA
      135 Fleet               1 <NA>        agecomp_expected <NA>        NA
      136 Fleet               1 <NA>        agecomp_expected <NA>        NA
      137 Fleet               1 <NA>        agecomp_expected <NA>        NA
      138 Fleet               1 <NA>        agecomp_expected <NA>        NA
      139 Fleet               1 <NA>        agecomp_expected <NA>        NA
      140 Fleet               1 <NA>        agecomp_expected <NA>        NA
      141 Fleet               1 <NA>        agecomp_expected <NA>        NA
      142 Fleet               1 <NA>        agecomp_expected <NA>        NA
      143 Fleet               1 <NA>        agecomp_expected <NA>        NA
      144 Fleet               1 <NA>        agecomp_expected <NA>        NA
      145 Fleet               1 <NA>        agecomp_expected <NA>        NA
      146 Fleet               1 <NA>        agecomp_expected <NA>        NA
      147 Fleet               1 <NA>        agecomp_expected <NA>        NA
      148 Fleet               1 <NA>        agecomp_expected <NA>        NA
      149 Fleet               1 <NA>        agecomp_expected <NA>        NA
      150 Fleet               1 <NA>        agecomp_expected <NA>        NA
      151 Fleet               1 <NA>        agecomp_expected <NA>        NA
      152 Fleet               1 <NA>        agecomp_expected <NA>        NA
      153 Fleet               1 <NA>        agecomp_expected <NA>        NA
      154 Fleet               1 <NA>        agecomp_expected <NA>        NA
      155 Fleet               1 <NA>        agecomp_expected <NA>        NA
      156 Fleet               1 <NA>        agecomp_expected <NA>        NA
      157 Fleet               1 <NA>        agecomp_expected <NA>        NA
      158 Fleet               1 <NA>        agecomp_expected <NA>        NA
      159 Fleet               1 <NA>        agecomp_expected <NA>        NA
      160 Fleet               1 <NA>        agecomp_expected <NA>        NA
      161 Fleet               1 <NA>        agecomp_expected <NA>        NA
      162 Fleet               1 <NA>        agecomp_expected <NA>        NA
      163 Fleet               1 <NA>        agecomp_expected <NA>        NA
      164 Fleet               1 <NA>        agecomp_expected <NA>        NA
      165 Fleet               1 <NA>        agecomp_expected <NA>        NA
      166 Fleet               1 <NA>        agecomp_expected <NA>        NA
      167 Fleet               1 <NA>        agecomp_expected <NA>        NA
      168 Fleet               1 <NA>        agecomp_expected <NA>        NA
      169 Fleet               1 <NA>        agecomp_expected <NA>        NA
      170 Fleet               1 <NA>        agecomp_expected <NA>        NA
      171 Fleet               1 <NA>        agecomp_expected <NA>        NA
      172 Fleet               1 <NA>        agecomp_expected <NA>        NA
      173 Fleet               1 <NA>        agecomp_expected <NA>        NA
      174 Fleet               1 <NA>        agecomp_expected <NA>        NA
      175 Fleet               1 <NA>        agecomp_expected <NA>        NA
      176 Fleet               1 <NA>        agecomp_expected <NA>        NA
      177 Fleet               1 <NA>        agecomp_expected <NA>        NA
      178 Fleet               1 <NA>        agecomp_expected <NA>        NA
      179 Fleet               1 <NA>        agecomp_expected <NA>        NA
      180 Fleet               1 <NA>        agecomp_expected <NA>        NA
      181 Fleet               1 <NA>        agecomp_expected <NA>        NA
      182 Fleet               1 <NA>        agecomp_expected <NA>        NA
      183 Fleet               1 <NA>        agecomp_expected <NA>        NA
      184 Fleet               1 <NA>        agecomp_expected <NA>        NA
      185 Fleet               1 <NA>        agecomp_expected <NA>        NA
      186 Fleet               1 <NA>        agecomp_expected <NA>        NA
      187 Fleet               1 <NA>        agecomp_expected <NA>        NA
      188 Fleet               1 <NA>        agecomp_expected <NA>        NA
      189 Fleet               1 <NA>        agecomp_expected <NA>        NA
      190 Fleet               1 <NA>        agecomp_expected <NA>        NA
      191 Fleet               1 <NA>        agecomp_expected <NA>        NA
      192 Fleet               1 <NA>        agecomp_expected <NA>        NA
      193 Fleet               1 <NA>        agecomp_expected <NA>        NA
      194 Fleet               1 <NA>        agecomp_expected <NA>        NA
      195 Fleet               1 <NA>        agecomp_expected <NA>        NA
      196 Fleet               1 <NA>        agecomp_expected <NA>        NA
      197 Fleet               1 <NA>        agecomp_expected <NA>        NA
      198 Fleet               1 <NA>        agecomp_expected <NA>        NA
      199 Fleet               1 <NA>        agecomp_expected <NA>        NA
      200 Fleet               1 <NA>        agecomp_expected <NA>        NA
      201 Fleet               1 <NA>        agecomp_expected <NA>        NA
      202 Fleet               1 <NA>        agecomp_expected <NA>        NA
      203 Fleet               1 <NA>        agecomp_expected <NA>        NA
      204 Fleet               1 <NA>        agecomp_expected <NA>        NA
      205 Fleet               1 <NA>        agecomp_expected <NA>        NA
      206 Fleet               1 <NA>        agecomp_expected <NA>        NA
      207 Fleet               1 <NA>        agecomp_expected <NA>        NA
      208 Fleet               1 <NA>        agecomp_expected <NA>        NA
      209 Fleet               1 <NA>        agecomp_expected <NA>        NA
      210 Fleet               1 <NA>        agecomp_expected <NA>        NA
      211 Fleet               1 <NA>        agecomp_expected <NA>        NA
      212 Fleet               1 <NA>        agecomp_expected <NA>        NA
      213 Fleet               1 <NA>        agecomp_expected <NA>        NA
      214 Fleet               1 <NA>        agecomp_expected <NA>        NA
      215 Fleet               1 <NA>        agecomp_expected <NA>        NA
      216 Fleet               1 <NA>        agecomp_expected <NA>        NA
      217 Fleet               1 <NA>        agecomp_expected <NA>        NA
      218 Fleet               1 <NA>        agecomp_expected <NA>        NA
      219 Fleet               1 <NA>        agecomp_expected <NA>        NA
      220 Fleet               1 <NA>        agecomp_expected <NA>        NA
      221 Fleet               1 <NA>        agecomp_expected <NA>        NA
      222 Fleet               1 <NA>        agecomp_expected <NA>        NA
      223 Fleet               1 <NA>        agecomp_expected <NA>        NA
      224 Fleet               1 <NA>        agecomp_expected <NA>        NA
      225 Fleet               1 <NA>        agecomp_expected <NA>        NA
      226 Fleet               1 <NA>        agecomp_expected <NA>        NA
      227 Fleet               1 <NA>        agecomp_expected <NA>        NA
      228 Fleet               1 <NA>        agecomp_expected <NA>        NA
      229 Fleet               1 <NA>        agecomp_expected <NA>        NA
      230 Fleet               1 <NA>        agecomp_expected <NA>        NA
      231 Fleet               1 <NA>        agecomp_expected <NA>        NA
      232 Fleet               1 <NA>        agecomp_expected <NA>        NA
      233 Fleet               1 <NA>        agecomp_expected <NA>        NA
      234 Fleet               1 <NA>        agecomp_expected <NA>        NA
      235 Fleet               1 <NA>        agecomp_expected <NA>        NA
      236 Fleet               1 <NA>        agecomp_expected <NA>        NA
      237 Fleet               1 <NA>        agecomp_expected <NA>        NA
      238 Fleet               1 <NA>        agecomp_expected <NA>        NA
      239 Fleet               1 <NA>        agecomp_expected <NA>        NA
      240 Fleet               1 <NA>        agecomp_expected <NA>        NA
      241 Fleet               1 <NA>        agecomp_expected <NA>        NA
      242 Fleet               1 <NA>        agecomp_expected <NA>        NA
      243 Fleet               1 <NA>        agecomp_expected <NA>        NA
      244 Fleet               1 <NA>        agecomp_expected <NA>        NA
      245 Fleet               1 <NA>        agecomp_expected <NA>        NA
      246 Fleet               1 <NA>        agecomp_expected <NA>        NA
      247 Fleet               1 <NA>        agecomp_expected <NA>        NA
      248 Fleet               1 <NA>        agecomp_expected <NA>        NA
      249 Fleet               1 <NA>        agecomp_expected <NA>        NA
      250 Fleet               1 <NA>        agecomp_expected <NA>        NA
      251 Fleet               1 <NA>        agecomp_expected <NA>        NA
      252 Fleet               1 <NA>        agecomp_expected <NA>        NA
      253 Fleet               1 <NA>        agecomp_expected <NA>        NA
      254 Fleet               1 <NA>        agecomp_expected <NA>        NA
      255 Fleet               1 <NA>        agecomp_expected <NA>        NA
      256 Fleet               1 <NA>        agecomp_expected <NA>        NA
      257 Fleet               1 <NA>        agecomp_expected <NA>        NA
      258 Fleet               1 <NA>        agecomp_expected <NA>        NA
      259 Fleet               1 <NA>        agecomp_expected <NA>        NA
      260 Fleet               1 <NA>        agecomp_expected <NA>        NA
      261 Fleet               1 <NA>        agecomp_expected <NA>        NA
      262 Fleet               1 <NA>        agecomp_expected <NA>        NA
      263 Fleet               1 <NA>        agecomp_expected <NA>        NA
      264 Fleet               1 <NA>        agecomp_expected <NA>        NA
      265 Fleet               1 <NA>        agecomp_expected <NA>        NA
      266 Fleet               1 <NA>        agecomp_expected <NA>        NA
      267 Fleet               1 <NA>        agecomp_expected <NA>        NA
      268 Fleet               1 <NA>        agecomp_expected <NA>        NA
      269 Fleet               1 <NA>        agecomp_expected <NA>        NA
      270 Fleet               1 <NA>        agecomp_expected <NA>        NA
      271 Fleet               1 <NA>        agecomp_expected <NA>        NA
      272 Fleet               1 <NA>        agecomp_expected <NA>        NA
      273 Fleet               1 <NA>        agecomp_expected <NA>        NA
      274 Fleet               1 <NA>        agecomp_expected <NA>        NA
      275 Fleet               1 <NA>        agecomp_expected <NA>        NA
      276 Fleet               1 <NA>        agecomp_expected <NA>        NA
      277 Fleet               1 <NA>        agecomp_expected <NA>        NA
      278 Fleet               1 <NA>        agecomp_expected <NA>        NA
      279 Fleet               1 <NA>        agecomp_expected <NA>        NA
      280 Fleet               1 <NA>        agecomp_expected <NA>        NA
      281 Fleet               1 <NA>        agecomp_expected <NA>        NA
      282 Fleet               1 <NA>        agecomp_expected <NA>        NA
      283 Fleet               1 <NA>        agecomp_expected <NA>        NA
      284 Fleet               1 <NA>        agecomp_expected <NA>        NA
      285 Fleet               1 <NA>        agecomp_expected <NA>        NA
      286 Fleet               1 <NA>        agecomp_expected <NA>        NA
      287 Fleet               1 <NA>        agecomp_expected <NA>        NA
      288 Fleet               1 <NA>        agecomp_expected <NA>        NA
      289 Fleet               1 <NA>        agecomp_expected <NA>        NA
      290 Fleet               1 <NA>        agecomp_expected <NA>        NA
      291 Fleet               1 <NA>        agecomp_expected <NA>        NA
      292 Fleet               1 <NA>        agecomp_expected <NA>        NA
      293 Fleet               1 <NA>        agecomp_expected <NA>        NA
      294 Fleet               1 <NA>        agecomp_expected <NA>        NA
      295 Fleet               1 <NA>        agecomp_expected <NA>        NA
      296 Fleet               1 <NA>        agecomp_expected <NA>        NA
      297 Fleet               1 <NA>        agecomp_expected <NA>        NA
      298 Fleet               1 <NA>        agecomp_expected <NA>        NA
      299 Fleet               1 <NA>        agecomp_expected <NA>        NA
      300 Fleet               1 <NA>        agecomp_expected <NA>        NA
      301 Fleet               1 <NA>        agecomp_expected <NA>        NA
      302 Fleet               1 <NA>        agecomp_expected <NA>        NA
      303 Fleet               1 <NA>        agecomp_expected <NA>        NA
      304 Fleet               1 <NA>        agecomp_expected <NA>        NA
      305 Fleet               1 <NA>        agecomp_expected <NA>        NA
      306 Fleet               1 <NA>        agecomp_expected <NA>        NA
      307 Fleet               1 <NA>        agecomp_expected <NA>        NA
      308 Fleet               1 <NA>        agecomp_expected <NA>        NA
      309 Fleet               1 <NA>        agecomp_expected <NA>        NA
      310 Fleet               1 <NA>        agecomp_expected <NA>        NA
      311 Fleet               1 <NA>        agecomp_expected <NA>        NA
      312 Fleet               1 <NA>        agecomp_expected <NA>        NA
      313 Fleet               1 <NA>        agecomp_expected <NA>        NA
      314 Fleet               1 <NA>        agecomp_expected <NA>        NA
      315 Fleet               1 <NA>        agecomp_expected <NA>        NA
      316 Fleet               1 <NA>        agecomp_expected <NA>        NA
      317 Fleet               1 <NA>        agecomp_expected <NA>        NA
      318 Fleet               1 <NA>        agecomp_expected <NA>        NA
      319 Fleet               1 <NA>        agecomp_expected <NA>        NA
      320 Fleet               1 <NA>        agecomp_expected <NA>        NA
          parameter_id fleet year_i age_i stratum_i length_i  input observed
                 <int> <chr>  <int> <int>     <int>    <int>  <dbl>    <dbl>
        1            4 NA         1    NA        NA       NA -4.66        NA
        2          287 NA         2    NA        NA       NA -3.60        NA
        3          288 NA         3    NA        NA       NA -3.10        NA
        4          289 NA         4    NA        NA       NA -2.80        NA
        5          290 NA         5    NA        NA       NA -3.02        NA
        6          291 NA         6    NA        NA       NA -2.44        NA
        7          292 NA         7    NA        NA       NA -2.43        NA
        8          293 NA         8    NA        NA       NA -1.68        NA
        9          294 NA         9    NA        NA       NA -2.22        NA
       10          295 NA        10    NA        NA       NA -2.02        NA
       11          296 NA        11    NA        NA       NA -1.89        NA
       12          297 NA        12    NA        NA       NA -1.82        NA
       13          298 NA        13    NA        NA       NA -2.15        NA
       14          299 NA        14    NA        NA       NA -1.78        NA
       15          300 NA        15    NA        NA       NA -1.71        NA
       16          301 NA        16    NA        NA       NA -1.82        NA
       17          302 NA        17    NA        NA       NA -1.16        NA
       18          303 NA        18    NA        NA       NA -1.36        NA
       19          304 NA        19    NA        NA       NA -1.37        NA
       20          305 NA        20    NA        NA       NA -1.38        NA
       21          306 NA        21    NA        NA       NA -1.05        NA
       22          307 NA        22    NA        NA       NA -1.37        NA
       23          308 NA        23    NA        NA       NA -0.871       NA
       24          309 NA        24    NA        NA       NA -1.06        NA
       25          310 NA        25    NA        NA       NA -1.07        NA
       26          311 NA        26    NA        NA       NA -1.16        NA
       27          312 NA        27    NA        NA       NA -1.18        NA
       28          313 NA        28    NA        NA       NA -0.840       NA
       29          314 NA        29    NA        NA       NA -1.11        NA
       30          315 NA        30    NA        NA       NA -0.694       NA
       31            3 NA        NA    NA        NA       NA  0           NA
       32           NA NA         1     1        NA       NA NA           14
       33           NA NA         1     2        NA       NA NA           20
       34           NA NA         1     3        NA       NA NA           23
       35           NA NA         1     4        NA       NA NA           30
       36           NA NA         1     5        NA       NA NA           20
       37           NA NA         1     6        NA       NA NA           10
       38           NA NA         1     7        NA       NA NA           22
       39           NA NA         1     8        NA       NA NA           11
       40           NA NA         1     9        NA       NA NA           12
       41           NA NA         1    10        NA       NA NA            6
       42           NA NA         1    11        NA       NA NA            7
       43           NA NA         1    12        NA       NA NA           25
       44           NA NA         2     1        NA       NA NA           21
       45           NA NA         2     2        NA       NA NA           19
       46           NA NA         2     3        NA       NA NA           22
       47           NA NA         2     4        NA       NA NA           18
       48           NA NA         2     5        NA       NA NA           31
       49           NA NA         2     6        NA       NA NA           11
       50           NA NA         2     7        NA       NA NA           13
       51           NA NA         2     8        NA       NA NA           17
       52           NA NA         2     9        NA       NA NA            9
       53           NA NA         2    10        NA       NA NA            7
       54           NA NA         2    11        NA       NA NA            5
       55           NA NA         2    12        NA       NA NA           27
       56           NA NA         3     1        NA       NA NA           11
       57           NA NA         3     2        NA       NA NA           30
       58           NA NA         3     3        NA       NA NA           21
       59           NA NA         3     4        NA       NA NA           23
       60           NA NA         3     5        NA       NA NA           19
       61           NA NA         3     6        NA       NA NA           18
       62           NA NA         3     7        NA       NA NA           14
       63           NA NA         3     8        NA       NA NA           17
       64           NA NA         3     9        NA       NA NA            6
       65           NA NA         3    10        NA       NA NA            9
       66           NA NA         3    11        NA       NA NA            6
       67           NA NA         3    12        NA       NA NA           26
       68           NA NA         4     1        NA       NA NA           10
       69           NA NA         4     2        NA       NA NA           11
       70           NA NA         4     3        NA       NA NA           44
       71           NA NA         4     4        NA       NA NA           20
       72           NA NA         4     5        NA       NA NA           20
       73           NA NA         4     6        NA       NA NA           21
       74           NA NA         4     7        NA       NA NA           13
       75           NA NA         4     8        NA       NA NA           11
       76           NA NA         4     9        NA       NA NA           13
       77           NA NA         4    10        NA       NA NA            6
       78           NA NA         4    11        NA       NA NA            3
       79           NA NA         4    12        NA       NA NA           28
       80           NA NA         5     1        NA       NA NA           31
       81           NA NA         5     2        NA       NA NA           10
       82           NA NA         5     3        NA       NA NA           32
       83           NA NA         5     4        NA       NA NA           34
       84           NA NA         5     5        NA       NA NA           18
       85           NA NA         5     6        NA       NA NA           19
       86           NA NA         5     7        NA       NA NA            9
       87           NA NA         5     8        NA       NA NA            9
       88           NA NA         5     9        NA       NA NA            6
       89           NA NA         5    10        NA       NA NA            4
       90           NA NA         5    11        NA       NA NA            6
       91           NA NA         5    12        NA       NA NA           22
       92           NA NA         6     1        NA       NA NA           16
       93           NA NA         6     2        NA       NA NA           37
       94           NA NA         6     3        NA       NA NA           14
       95           NA NA         6     4        NA       NA NA           18
       96           NA NA         6     5        NA       NA NA           29
       97           NA NA         6     6        NA       NA NA           15
       98           NA NA         6     7        NA       NA NA           12
       99           NA NA         6     8        NA       NA NA           13
      100           NA NA         6     9        NA       NA NA            8
      101           NA NA         6    10        NA       NA NA            9
      102           NA NA         6    11        NA       NA NA            5
      103           NA NA         6    12        NA       NA NA           24
      104           NA NA         7     1        NA       NA NA           11
      105           NA NA         7     2        NA       NA NA           26
      106           NA NA         7     3        NA       NA NA           44
      107           NA NA         7     4        NA       NA NA           17
      108           NA NA         7     5        NA       NA NA           19
      109           NA NA         7     6        NA       NA NA           28
      110           NA NA         7     7        NA       NA NA            7
      111           NA NA         7     8        NA       NA NA            8
      112           NA NA         7     9        NA       NA NA            7
      113           NA NA         7    10        NA       NA NA            4
      114           NA NA         7    11        NA       NA NA            5
      115           NA NA         7    12        NA       NA NA           24
      116           NA NA         8     1        NA       NA NA           18
      117           NA NA         8     2        NA       NA NA           15
      118           NA NA         8     3        NA       NA NA           44
      119           NA NA         8     4        NA       NA NA           40
      120           NA NA         8     5        NA       NA NA           12
      121           NA NA         8     6        NA       NA NA            8
      122           NA NA         8     7        NA       NA NA           14
      123           NA NA         8     8        NA       NA NA           13
      124           NA NA         8     9        NA       NA NA            5
      125           NA NA         8    10        NA       NA NA            5
      126           NA NA         8    11        NA       NA NA            5
      127           NA NA         8    12        NA       NA NA           21
      128           NA NA         9     1        NA       NA NA           14
      129           NA NA         9     2        NA       NA NA           32
      130           NA NA         9     3        NA       NA NA           27
      131           NA NA         9     4        NA       NA NA           35
      132           NA NA         9     5        NA       NA NA           31
      133           NA NA         9     6        NA       NA NA            9
      134           NA NA         9     7        NA       NA NA            9
      135           NA NA         9     8        NA       NA NA           12
      136           NA NA         9     9        NA       NA NA            5
      137           NA NA         9    10        NA       NA NA            5
      138           NA NA         9    11        NA       NA NA            3
      139           NA NA         9    12        NA       NA NA           18
      140           NA NA        10     1        NA       NA NA           15
      141           NA NA        10     2        NA       NA NA           21
      142           NA NA        10     3        NA       NA NA           43
      143           NA NA        10     4        NA       NA NA           20
      144           NA NA        10     5        NA       NA NA           22
      145           NA NA        10     6        NA       NA NA           21
      146           NA NA        10     7        NA       NA NA            7
      147           NA NA        10     8        NA       NA NA            8
      148           NA NA        10     9        NA       NA NA           13
      149           NA NA        10    10        NA       NA NA            4
      150           NA NA        10    11        NA       NA NA            4
      151           NA NA        10    12        NA       NA NA           22
      152           NA NA        11     1        NA       NA NA           15
      153           NA NA        11     2        NA       NA NA           34
      154           NA NA        11     3        NA       NA NA           23
      155           NA NA        11     4        NA       NA NA           43
      156           NA NA        11     5        NA       NA NA           15
      157           NA NA        11     6        NA       NA NA           23
      158           NA NA        11     7        NA       NA NA           21
      159           NA NA        11     8        NA       NA NA            5
      160           NA NA        11     9        NA       NA NA            3
      161           NA NA        11    10        NA       NA NA            5
      162           NA NA        11    11        NA       NA NA            2
      163           NA NA        11    12        NA       NA NA           11
      164           NA NA        12     1        NA       NA NA           11
      165           NA NA        12     2        NA       NA NA           32
      166           NA NA        12     3        NA       NA NA           30
      167           NA NA        12     4        NA       NA NA           26
      168           NA NA        12     5        NA       NA NA           33
      169           NA NA        12     6        NA       NA NA           14
      170           NA NA        12     7        NA       NA NA           13
      171           NA NA        12     8        NA       NA NA           21
      172           NA NA        12     9        NA       NA NA            4
      173           NA NA        12    10        NA       NA NA            3
      174           NA NA        12    11        NA       NA NA            4
      175           NA NA        12    12        NA       NA NA            9
      176           NA NA        13     1        NA       NA NA           22
      177           NA NA        13     2        NA       NA NA           21
      178           NA NA        13     3        NA       NA NA           23
      179           NA NA        13     4        NA       NA NA           28
      180           NA NA        13     5        NA       NA NA           23
      181           NA NA        13     6        NA       NA NA           18
      182           NA NA        13     7        NA       NA NA           15
      183           NA NA        13     8        NA       NA NA           18
      184           NA NA        13     9        NA       NA NA           16
      185           NA NA        13    10        NA       NA NA            3
      186           NA NA        13    11        NA       NA NA            3
      187           NA NA        13    12        NA       NA NA           10
      188           NA NA        14     1        NA       NA NA           23
      189           NA NA        14     2        NA       NA NA           28
      190           NA NA        14     3        NA       NA NA           17
      191           NA NA        14     4        NA       NA NA           41
      192           NA NA        14     5        NA       NA NA           24
      193           NA NA        14     6        NA       NA NA           10
      194           NA NA        14     7        NA       NA NA           10
      195           NA NA        14     8        NA       NA NA            8
      196           NA NA        14     9        NA       NA NA            9
      197           NA NA        14    10        NA       NA NA           12
      198           NA NA        14    11        NA       NA NA            5
      199           NA NA        14    12        NA       NA NA           13
      200           NA NA        15     1        NA       NA NA           14
      201           NA NA        15     2        NA       NA NA           38
      202           NA NA        15     3        NA       NA NA           39
      203           NA NA        15     4        NA       NA NA           16
      204           NA NA        15     5        NA       NA NA           28
      205           NA NA        15     6        NA       NA NA           21
      206           NA NA        15     7        NA       NA NA           10
      207           NA NA        15     8        NA       NA NA           11
      208           NA NA        15     9        NA       NA NA            5
      209           NA NA        15    10        NA       NA NA            4
      210           NA NA        15    11        NA       NA NA            0
      211           NA NA        15    12        NA       NA NA           14
      212           NA NA        16     1        NA       NA NA           29
      213           NA NA        16     2        NA       NA NA           25
      214           NA NA        16     3        NA       NA NA           39
      215           NA NA        16     4        NA       NA NA           25
      216           NA NA        16     5        NA       NA NA           17
      217           NA NA        16     6        NA       NA NA           13
      218           NA NA        16     7        NA       NA NA           12
      219           NA NA        16     8        NA       NA NA            8
      220           NA NA        16     9        NA       NA NA            7
      221           NA NA        16    10        NA       NA NA            3
      222           NA NA        16    11        NA       NA NA            7
      223           NA NA        16    12        NA       NA NA           15
      224           NA NA        17     1        NA       NA NA           33
      225           NA NA        17     2        NA       NA NA           38
      226           NA NA        17     3        NA       NA NA           28
      227           NA NA        17     4        NA       NA NA           29
      228           NA NA        17     5        NA       NA NA           24
      229           NA NA        17     6        NA       NA NA            6
      230           NA NA        17     7        NA       NA NA           10
      231           NA NA        17     8        NA       NA NA            1
      232           NA NA        17     9        NA       NA NA            6
      233           NA NA        17    10        NA       NA NA            5
      234           NA NA        17    11        NA       NA NA            2
      235           NA NA        17    12        NA       NA NA           18
      236           NA NA        18     1        NA       NA NA           17
      237           NA NA        18     2        NA       NA NA           40
      238           NA NA        18     3        NA       NA NA           50
      239           NA NA        18     4        NA       NA NA           25
      240           NA NA        18     5        NA       NA NA           17
      241           NA NA        18     6        NA       NA NA           13
      242           NA NA        18     7        NA       NA NA           11
      243           NA NA        18     8        NA       NA NA            8
      244           NA NA        18     9        NA       NA NA            7
      245           NA NA        18    10        NA       NA NA            1
      246           NA NA        18    11        NA       NA NA            4
      247           NA NA        18    12        NA       NA NA            7
      248           NA NA        19     1        NA       NA NA           26
      249           NA NA        19     2        NA       NA NA           24
      250           NA NA        19     3        NA       NA NA           37
      251           NA NA        19     4        NA       NA NA           34
      252           NA NA        19     5        NA       NA NA           22
      253           NA NA        19     6        NA       NA NA           23
      254           NA NA        19     7        NA       NA NA            8
      255           NA NA        19     8        NA       NA NA            6
      256           NA NA        19     9        NA       NA NA            3
      257           NA NA        19    10        NA       NA NA            5
      258           NA NA        19    11        NA       NA NA            4
      259           NA NA        19    12        NA       NA NA            8
      260           NA NA        20     1        NA       NA NA            9
      261           NA NA        20     2        NA       NA NA           31
      262           NA NA        20     3        NA       NA NA           33
      263           NA NA        20     4        NA       NA NA           51
      264           NA NA        20     5        NA       NA NA           29
      265           NA NA        20     6        NA       NA NA           12
      266           NA NA        20     7        NA       NA NA           11
      267           NA NA        20     8        NA       NA NA           10
      268           NA NA        20     9        NA       NA NA            5
      269           NA NA        20    10        NA       NA NA            2
      270           NA NA        20    11        NA       NA NA            5
      271           NA NA        20    12        NA       NA NA            2
      272           NA NA        21     1        NA       NA NA           49
      273           NA NA        21     2        NA       NA NA           15
      274           NA NA        21     3        NA       NA NA           38
      275           NA NA        21     4        NA       NA NA           23
      276           NA NA        21     5        NA       NA NA           25
      277           NA NA        21     6        NA       NA NA           17
      278           NA NA        21     7        NA       NA NA           10
      279           NA NA        21     8        NA       NA NA            8
      280           NA NA        21     9        NA       NA NA            2
      281           NA NA        21    10        NA       NA NA            1
      282           NA NA        21    11        NA       NA NA            5
      283           NA NA        21    12        NA       NA NA            7
      284           NA NA        22     1        NA       NA NA           40
      285           NA NA        22     2        NA       NA NA           67
      286           NA NA        22     3        NA       NA NA           14
      287           NA NA        22     4        NA       NA NA           23
      288           NA NA        22     5        NA       NA NA           15
      289           NA NA        22     6        NA       NA NA            6
      290           NA NA        22     7        NA       NA NA           14
      291           NA NA        22     8        NA       NA NA            4
      292           NA NA        22     9        NA       NA NA            5
      293           NA NA        22    10        NA       NA NA            4
      294           NA NA        22    11        NA       NA NA            1
      295           NA NA        22    12        NA       NA NA            7
      296           NA NA        23     1        NA       NA NA           22
      297           NA NA        23     2        NA       NA NA           51
      298           NA NA        23     3        NA       NA NA           56
      299           NA NA        23     4        NA       NA NA           16
      300           NA NA        23     5        NA       NA NA           11
      301           NA NA        23     6        NA       NA NA            7
      302           NA NA        23     7        NA       NA NA           18
      303           NA NA        23     8        NA       NA NA            5
      304           NA NA        23     9        NA       NA NA            1
      305           NA NA        23    10        NA       NA NA            3
      306           NA NA        23    11        NA       NA NA            5
      307           NA NA        23    12        NA       NA NA            5
      308           NA NA        24     1        NA       NA NA           24
      309           NA NA        24     2        NA       NA NA           38
      310           NA NA        24     3        NA       NA NA           48
      311           NA NA        24     4        NA       NA NA           36
      312           NA NA        24     5        NA       NA NA           14
      313           NA NA        24     6        NA       NA NA           15
      314           NA NA        24     7        NA       NA NA            4
      315           NA NA        24     8        NA       NA NA            6
      316           NA NA        24     9        NA       NA NA            8
      317           NA NA        24    10        NA       NA NA            1
      318           NA NA        24    11        NA       NA NA            4
      319           NA NA        24    12        NA       NA NA            2
      320           NA NA        25     1        NA       NA NA           36
          estimation_type  distribution input_type  lpdf log_sd
          <chr>            <chr>        <chr>      <dbl>  <dbl>
        1 fixed_effects    <NA>         <NA>         NA      NA
        2 fixed_effects    <NA>         <NA>         NA      NA
        3 fixed_effects    <NA>         <NA>         NA      NA
        4 fixed_effects    <NA>         <NA>         NA      NA
        5 fixed_effects    <NA>         <NA>         NA      NA
        6 fixed_effects    <NA>         <NA>         NA      NA
        7 fixed_effects    <NA>         <NA>         NA      NA
        8 fixed_effects    <NA>         <NA>         NA      NA
        9 fixed_effects    <NA>         <NA>         NA      NA
       10 fixed_effects    <NA>         <NA>         NA      NA
       11 fixed_effects    <NA>         <NA>         NA      NA
       12 fixed_effects    <NA>         <NA>         NA      NA
       13 fixed_effects    <NA>         <NA>         NA      NA
       14 fixed_effects    <NA>         <NA>         NA      NA
       15 fixed_effects    <NA>         <NA>         NA      NA
       16 fixed_effects    <NA>         <NA>         NA      NA
       17 fixed_effects    <NA>         <NA>         NA      NA
       18 fixed_effects    <NA>         <NA>         NA      NA
       19 fixed_effects    <NA>         <NA>         NA      NA
       20 fixed_effects    <NA>         <NA>         NA      NA
       21 fixed_effects    <NA>         <NA>         NA      NA
       22 fixed_effects    <NA>         <NA>         NA      NA
       23 fixed_effects    <NA>         <NA>         NA      NA
       24 fixed_effects    <NA>         <NA>         NA      NA
       25 fixed_effects    <NA>         <NA>         NA      NA
       26 fixed_effects    <NA>         <NA>         NA      NA
       27 fixed_effects    <NA>         <NA>         NA      NA
       28 fixed_effects    <NA>         <NA>         NA      NA
       29 fixed_effects    <NA>         <NA>         NA      NA
       30 fixed_effects    <NA>         <NA>         NA      NA
       31 constant         <NA>         <NA>         NA      NA
       32 derived_quantity multinomial  data       -826.     NA
       33 derived_quantity multinomial  data       -826.     NA
       34 derived_quantity multinomial  data       -826.     NA
       35 derived_quantity multinomial  data       -826.     NA
       36 derived_quantity multinomial  data       -826.     NA
       37 derived_quantity multinomial  data       -826.     NA
       38 derived_quantity multinomial  data       -826.     NA
       39 derived_quantity multinomial  data       -826.     NA
       40 derived_quantity multinomial  data       -826.     NA
       41 derived_quantity multinomial  data       -826.     NA
       42 derived_quantity multinomial  data       -826.     NA
       43 derived_quantity multinomial  data       -826.     NA
       44 derived_quantity multinomial  data       -826.     NA
       45 derived_quantity multinomial  data       -826.     NA
       46 derived_quantity multinomial  data       -826.     NA
       47 derived_quantity multinomial  data       -826.     NA
       48 derived_quantity multinomial  data       -826.     NA
       49 derived_quantity multinomial  data       -826.     NA
       50 derived_quantity multinomial  data       -826.     NA
       51 derived_quantity multinomial  data       -826.     NA
       52 derived_quantity multinomial  data       -826.     NA
       53 derived_quantity multinomial  data       -826.     NA
       54 derived_quantity multinomial  data       -826.     NA
       55 derived_quantity multinomial  data       -826.     NA
       56 derived_quantity multinomial  data       -826.     NA
       57 derived_quantity multinomial  data       -826.     NA
       58 derived_quantity multinomial  data       -826.     NA
       59 derived_quantity multinomial  data       -826.     NA
       60 derived_quantity multinomial  data       -826.     NA
       61 derived_quantity multinomial  data       -826.     NA
       62 derived_quantity multinomial  data       -826.     NA
       63 derived_quantity multinomial  data       -826.     NA
       64 derived_quantity multinomial  data       -826.     NA
       65 derived_quantity multinomial  data       -826.     NA
       66 derived_quantity multinomial  data       -826.     NA
       67 derived_quantity multinomial  data       -826.     NA
       68 derived_quantity multinomial  data       -826.     NA
       69 derived_quantity multinomial  data       -826.     NA
       70 derived_quantity multinomial  data       -826.     NA
       71 derived_quantity multinomial  data       -826.     NA
       72 derived_quantity multinomial  data       -826.     NA
       73 derived_quantity multinomial  data       -826.     NA
       74 derived_quantity multinomial  data       -826.     NA
       75 derived_quantity multinomial  data       -826.     NA
       76 derived_quantity multinomial  data       -826.     NA
       77 derived_quantity multinomial  data       -826.     NA
       78 derived_quantity multinomial  data       -826.     NA
       79 derived_quantity multinomial  data       -826.     NA
       80 derived_quantity multinomial  data       -826.     NA
       81 derived_quantity multinomial  data       -826.     NA
       82 derived_quantity multinomial  data       -826.     NA
       83 derived_quantity multinomial  data       -826.     NA
       84 derived_quantity multinomial  data       -826.     NA
       85 derived_quantity multinomial  data       -826.     NA
       86 derived_quantity multinomial  data       -826.     NA
       87 derived_quantity multinomial  data       -826.     NA
       88 derived_quantity multinomial  data       -826.     NA
       89 derived_quantity multinomial  data       -826.     NA
       90 derived_quantity multinomial  data       -826.     NA
       91 derived_quantity multinomial  data       -826.     NA
       92 derived_quantity multinomial  data       -826.     NA
       93 derived_quantity multinomial  data       -826.     NA
       94 derived_quantity multinomial  data       -826.     NA
       95 derived_quantity multinomial  data       -826.     NA
       96 derived_quantity multinomial  data       -826.     NA
       97 derived_quantity multinomial  data       -826.     NA
       98 derived_quantity multinomial  data       -826.     NA
       99 derived_quantity multinomial  data       -826.     NA
      100 derived_quantity multinomial  data       -826.     NA
      101 derived_quantity multinomial  data       -826.     NA
      102 derived_quantity multinomial  data       -826.     NA
      103 derived_quantity multinomial  data       -826.     NA
      104 derived_quantity multinomial  data       -826.     NA
      105 derived_quantity multinomial  data       -826.     NA
      106 derived_quantity multinomial  data       -826.     NA
      107 derived_quantity multinomial  data       -826.     NA
      108 derived_quantity multinomial  data       -826.     NA
      109 derived_quantity multinomial  data       -826.     NA
      110 derived_quantity multinomial  data       -826.     NA
      111 derived_quantity multinomial  data       -826.     NA
      112 derived_quantity multinomial  data       -826.     NA
      113 derived_quantity multinomial  data       -826.     NA
      114 derived_quantity multinomial  data       -826.     NA
      115 derived_quantity multinomial  data       -826.     NA
      116 derived_quantity multinomial  data       -826.     NA
      117 derived_quantity multinomial  data       -826.     NA
      118 derived_quantity multinomial  data       -826.     NA
      119 derived_quantity multinomial  data       -826.     NA
      120 derived_quantity multinomial  data       -826.     NA
      121 derived_quantity multinomial  data       -826.     NA
      122 derived_quantity multinomial  data       -826.     NA
      123 derived_quantity multinomial  data       -826.     NA
      124 derived_quantity multinomial  data       -826.     NA
      125 derived_quantity multinomial  data       -826.     NA
      126 derived_quantity multinomial  data       -826.     NA
      127 derived_quantity multinomial  data       -826.     NA
      128 derived_quantity multinomial  data       -826.     NA
      129 derived_quantity multinomial  data       -826.     NA
      130 derived_quantity multinomial  data       -826.     NA
      131 derived_quantity multinomial  data       -826.     NA
      132 derived_quantity multinomial  data       -826.     NA
      133 derived_quantity multinomial  data       -826.     NA
      134 derived_quantity multinomial  data       -826.     NA
      135 derived_quantity multinomial  data       -826.     NA
      136 derived_quantity multinomial  data       -826.     NA
      137 derived_quantity multinomial  data       -826.     NA
      138 derived_quantity multinomial  data       -826.     NA
      139 derived_quantity multinomial  data       -826.     NA
      140 derived_quantity multinomial  data       -826.     NA
      141 derived_quantity multinomial  data       -826.     NA
      142 derived_quantity multinomial  data       -826.     NA
      143 derived_quantity multinomial  data       -826.     NA
      144 derived_quantity multinomial  data       -826.     NA
      145 derived_quantity multinomial  data       -826.     NA
      146 derived_quantity multinomial  data       -826.     NA
      147 derived_quantity multinomial  data       -826.     NA
      148 derived_quantity multinomial  data       -826.     NA
      149 derived_quantity multinomial  data       -826.     NA
      150 derived_quantity multinomial  data       -826.     NA
      151 derived_quantity multinomial  data       -826.     NA
      152 derived_quantity multinomial  data       -826.     NA
      153 derived_quantity multinomial  data       -826.     NA
      154 derived_quantity multinomial  data       -826.     NA
      155 derived_quantity multinomial  data       -826.     NA
      156 derived_quantity multinomial  data       -826.     NA
      157 derived_quantity multinomial  data       -826.     NA
      158 derived_quantity multinomial  data       -826.     NA
      159 derived_quantity multinomial  data       -826.     NA
      160 derived_quantity multinomial  data       -826.     NA
      161 derived_quantity multinomial  data       -826.     NA
      162 derived_quantity multinomial  data       -826.     NA
      163 derived_quantity multinomial  data       -826.     NA
      164 derived_quantity multinomial  data       -826.     NA
      165 derived_quantity multinomial  data       -826.     NA
      166 derived_quantity multinomial  data       -826.     NA
      167 derived_quantity multinomial  data       -826.     NA
      168 derived_quantity multinomial  data       -826.     NA
      169 derived_quantity multinomial  data       -826.     NA
      170 derived_quantity multinomial  data       -826.     NA
      171 derived_quantity multinomial  data       -826.     NA
      172 derived_quantity multinomial  data       -826.     NA
      173 derived_quantity multinomial  data       -826.     NA
      174 derived_quantity multinomial  data       -826.     NA
      175 derived_quantity multinomial  data       -826.     NA
      176 derived_quantity multinomial  data       -826.     NA
      177 derived_quantity multinomial  data       -826.     NA
      178 derived_quantity multinomial  data       -826.     NA
      179 derived_quantity multinomial  data       -826.     NA
      180 derived_quantity multinomial  data       -826.     NA
      181 derived_quantity multinomial  data       -826.     NA
      182 derived_quantity multinomial  data       -826.     NA
      183 derived_quantity multinomial  data       -826.     NA
      184 derived_quantity multinomial  data       -826.     NA
      185 derived_quantity multinomial  data       -826.     NA
      186 derived_quantity multinomial  data       -826.     NA
      187 derived_quantity multinomial  data       -826.     NA
      188 derived_quantity multinomial  data       -826.     NA
      189 derived_quantity multinomial  data       -826.     NA
      190 derived_quantity multinomial  data       -826.     NA
      191 derived_quantity multinomial  data       -826.     NA
      192 derived_quantity multinomial  data       -826.     NA
      193 derived_quantity multinomial  data       -826.     NA
      194 derived_quantity multinomial  data       -826.     NA
      195 derived_quantity multinomial  data       -826.     NA
      196 derived_quantity multinomial  data       -826.     NA
      197 derived_quantity multinomial  data       -826.     NA
      198 derived_quantity multinomial  data       -826.     NA
      199 derived_quantity multinomial  data       -826.     NA
      200 derived_quantity multinomial  data       -826.     NA
      201 derived_quantity multinomial  data       -826.     NA
      202 derived_quantity multinomial  data       -826.     NA
      203 derived_quantity multinomial  data       -826.     NA
      204 derived_quantity multinomial  data       -826.     NA
      205 derived_quantity multinomial  data       -826.     NA
      206 derived_quantity multinomial  data       -826.     NA
      207 derived_quantity multinomial  data       -826.     NA
      208 derived_quantity multinomial  data       -826.     NA
      209 derived_quantity multinomial  data       -826.     NA
      210 derived_quantity multinomial  data       -826.     NA
      211 derived_quantity multinomial  data       -826.     NA
      212 derived_quantity multinomial  data       -826.     NA
      213 derived_quantity multinomial  data       -826.     NA
      214 derived_quantity multinomial  data       -826.     NA
      215 derived_quantity multinomial  data       -826.     NA
      216 derived_quantity multinomial  data       -826.     NA
      217 derived_quantity multinomial  data       -826.     NA
      218 derived_quantity multinomial  data       -826.     NA
      219 derived_quantity multinomial  data       -826.     NA
      220 derived_quantity multinomial  data       -826.     NA
      221 derived_quantity multinomial  data       -826.     NA
      222 derived_quantity multinomial  data       -826.     NA
      223 derived_quantity multinomial  data       -826.     NA
      224 derived_quantity multinomial  data       -826.     NA
      225 derived_quantity multinomial  data       -826.     NA
      226 derived_quantity multinomial  data       -826.     NA
      227 derived_quantity multinomial  data       -826.     NA
      228 derived_quantity multinomial  data       -826.     NA
      229 derived_quantity multinomial  data       -826.     NA
      230 derived_quantity multinomial  data       -826.     NA
      231 derived_quantity multinomial  data       -826.     NA
      232 derived_quantity multinomial  data       -826.     NA
      233 derived_quantity multinomial  data       -826.     NA
      234 derived_quantity multinomial  data       -826.     NA
      235 derived_quantity multinomial  data       -826.     NA
      236 derived_quantity multinomial  data       -826.     NA
      237 derived_quantity multinomial  data       -826.     NA
      238 derived_quantity multinomial  data       -826.     NA
      239 derived_quantity multinomial  data       -826.     NA
      240 derived_quantity multinomial  data       -826.     NA
      241 derived_quantity multinomial  data       -826.     NA
      242 derived_quantity multinomial  data       -826.     NA
      243 derived_quantity multinomial  data       -826.     NA
      244 derived_quantity multinomial  data       -826.     NA
      245 derived_quantity multinomial  data       -826.     NA
      246 derived_quantity multinomial  data       -826.     NA
      247 derived_quantity multinomial  data       -826.     NA
      248 derived_quantity multinomial  data       -826.     NA
      249 derived_quantity multinomial  data       -826.     NA
      250 derived_quantity multinomial  data       -826.     NA
      251 derived_quantity multinomial  data       -826.     NA
      252 derived_quantity multinomial  data       -826.     NA
      253 derived_quantity multinomial  data       -826.     NA
      254 derived_quantity multinomial  data       -826.     NA
      255 derived_quantity multinomial  data       -826.     NA
      256 derived_quantity multinomial  data       -826.     NA
      257 derived_quantity multinomial  data       -826.     NA
      258 derived_quantity multinomial  data       -826.     NA
      259 derived_quantity multinomial  data       -826.     NA
      260 derived_quantity multinomial  data       -826.     NA
      261 derived_quantity multinomial  data       -826.     NA
      262 derived_quantity multinomial  data       -826.     NA
      263 derived_quantity multinomial  data       -826.     NA
      264 derived_quantity multinomial  data       -826.     NA
      265 derived_quantity multinomial  data       -826.     NA
      266 derived_quantity multinomial  data       -826.     NA
      267 derived_quantity multinomial  data       -826.     NA
      268 derived_quantity multinomial  data       -826.     NA
      269 derived_quantity multinomial  data       -826.     NA
      270 derived_quantity multinomial  data       -826.     NA
      271 derived_quantity multinomial  data       -826.     NA
      272 derived_quantity multinomial  data       -826.     NA
      273 derived_quantity multinomial  data       -826.     NA
      274 derived_quantity multinomial  data       -826.     NA
      275 derived_quantity multinomial  data       -826.     NA
      276 derived_quantity multinomial  data       -826.     NA
      277 derived_quantity multinomial  data       -826.     NA
      278 derived_quantity multinomial  data       -826.     NA
      279 derived_quantity multinomial  data       -826.     NA
      280 derived_quantity multinomial  data       -826.     NA
      281 derived_quantity multinomial  data       -826.     NA
      282 derived_quantity multinomial  data       -826.     NA
      283 derived_quantity multinomial  data       -826.     NA
      284 derived_quantity multinomial  data       -826.     NA
      285 derived_quantity multinomial  data       -826.     NA
      286 derived_quantity multinomial  data       -826.     NA
      287 derived_quantity multinomial  data       -826.     NA
      288 derived_quantity multinomial  data       -826.     NA
      289 derived_quantity multinomial  data       -826.     NA
      290 derived_quantity multinomial  data       -826.     NA
      291 derived_quantity multinomial  data       -826.     NA
      292 derived_quantity multinomial  data       -826.     NA
      293 derived_quantity multinomial  data       -826.     NA
      294 derived_quantity multinomial  data       -826.     NA
      295 derived_quantity multinomial  data       -826.     NA
      296 derived_quantity multinomial  data       -826.     NA
      297 derived_quantity multinomial  data       -826.     NA
      298 derived_quantity multinomial  data       -826.     NA
      299 derived_quantity multinomial  data       -826.     NA
      300 derived_quantity multinomial  data       -826.     NA
      301 derived_quantity multinomial  data       -826.     NA
      302 derived_quantity multinomial  data       -826.     NA
      303 derived_quantity multinomial  data       -826.     NA
      304 derived_quantity multinomial  data       -826.     NA
      305 derived_quantity multinomial  data       -826.     NA
      306 derived_quantity multinomial  data       -826.     NA
      307 derived_quantity multinomial  data       -826.     NA
      308 derived_quantity multinomial  data       -826.     NA
      309 derived_quantity multinomial  data       -826.     NA
      310 derived_quantity multinomial  data       -826.     NA
      311 derived_quantity multinomial  data       -826.     NA
      312 derived_quantity multinomial  data       -826.     NA
      313 derived_quantity multinomial  data       -826.     NA
      314 derived_quantity multinomial  data       -826.     NA
      315 derived_quantity multinomial  data       -826.     NA
      316 derived_quantity multinomial  data       -826.     NA
      317 derived_quantity multinomial  data       -826.     NA
      318 derived_quantity multinomial  data       -826.     NA
      319 derived_quantity multinomial  data       -826.     NA
      320 derived_quantity multinomial  data       -826.     NA
      # i 19,496 more rows

