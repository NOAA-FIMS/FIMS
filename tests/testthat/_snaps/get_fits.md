# get_fits() works with deterministic run

    Code
      print(dplyr::select(get_fits(deterministic_results), -expected, -log_like), n = 320,
      width = Inf)
    Output
      # A tibble: 2,160 x 17
          module_name module_id label    data_id fleet_name unit  uncertainty   age
          <chr>           <int> <chr>      <int> <chr>      <chr>       <dbl> <int>
        1 data                1 Landings      NA <NA>       <NA>           NA    NA
        2 data                1 Landings      NA <NA>       <NA>           NA    NA
        3 data                1 Landings      NA <NA>       <NA>           NA    NA
        4 data                1 Landings      NA <NA>       <NA>           NA    NA
        5 data                1 Landings      NA <NA>       <NA>           NA    NA
        6 data                1 Landings      NA <NA>       <NA>           NA    NA
        7 data                1 Landings      NA <NA>       <NA>           NA    NA
        8 data                1 Landings      NA <NA>       <NA>           NA    NA
        9 data                1 Landings      NA <NA>       <NA>           NA    NA
       10 data                1 Landings      NA <NA>       <NA>           NA    NA
       11 data                1 Landings      NA <NA>       <NA>           NA    NA
       12 data                1 Landings      NA <NA>       <NA>           NA    NA
       13 data                1 Landings      NA <NA>       <NA>           NA    NA
       14 data                1 Landings      NA <NA>       <NA>           NA    NA
       15 data                1 Landings      NA <NA>       <NA>           NA    NA
       16 data                1 Landings      NA <NA>       <NA>           NA    NA
       17 data                1 Landings      NA <NA>       <NA>           NA    NA
       18 data                1 Landings      NA <NA>       <NA>           NA    NA
       19 data                1 Landings      NA <NA>       <NA>           NA    NA
       20 data                1 Landings      NA <NA>       <NA>           NA    NA
       21 data                1 Landings      NA <NA>       <NA>           NA    NA
       22 data                1 Landings      NA <NA>       <NA>           NA    NA
       23 data                1 Landings      NA <NA>       <NA>           NA    NA
       24 data                1 Landings      NA <NA>       <NA>           NA    NA
       25 data                1 Landings      NA <NA>       <NA>           NA    NA
       26 data                1 Landings      NA <NA>       <NA>           NA    NA
       27 data                1 Landings      NA <NA>       <NA>           NA    NA
       28 data                1 Landings      NA <NA>       <NA>           NA    NA
       29 data                1 Landings      NA <NA>       <NA>           NA    NA
       30 data                1 Landings      NA <NA>       <NA>           NA    NA
       31 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       32 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       33 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       34 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       35 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       36 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       37 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       38 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       39 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       40 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       41 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       42 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       43 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       44 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       45 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       46 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       47 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       48 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       49 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       50 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       51 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       52 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       53 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       54 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       55 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       56 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       57 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       58 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       59 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       60 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       61 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       62 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       63 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       64 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       65 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       66 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       67 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       68 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       69 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       70 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       71 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       72 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       73 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       74 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       75 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       76 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       77 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       78 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       79 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       80 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       81 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       82 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       83 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       84 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       85 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       86 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       87 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       88 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       89 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       90 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       91 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       92 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       93 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       94 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       95 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       96 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       97 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       98 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       99 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      100 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      101 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      102 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      103 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      104 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      105 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      106 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      107 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      108 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      109 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      110 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      111 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      112 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      113 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      114 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      115 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      116 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      117 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      118 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      119 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      120 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      121 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      122 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      123 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      124 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      125 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      126 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      127 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      128 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      129 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      130 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      131 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      132 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      133 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      134 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      135 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      136 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      137 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      138 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      139 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      140 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      141 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      142 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      143 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      144 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      145 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      146 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      147 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      148 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      149 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      150 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      151 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      152 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      153 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      154 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      155 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      156 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      157 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      158 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      159 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      160 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      161 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      162 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      163 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      164 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      165 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      166 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      167 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      168 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      169 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      170 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      171 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      172 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      173 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      174 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      175 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      176 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      177 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      178 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      179 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      180 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      181 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      182 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      183 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      184 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      185 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      186 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      187 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      188 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      189 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      190 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      191 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      192 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      193 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      194 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      195 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      196 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      197 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      198 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      199 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      200 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      201 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      202 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      203 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      204 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      205 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      206 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      207 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      208 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      209 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      210 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      211 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      212 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      213 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      214 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      215 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      216 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      217 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      218 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      219 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      220 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      221 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      222 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      223 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      224 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      225 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      226 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      227 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      228 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      229 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      230 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      231 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      232 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      233 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      234 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      235 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      236 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      237 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      238 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      239 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      240 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      241 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      242 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      243 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      244 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      245 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      246 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      247 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      248 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      249 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      250 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      251 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      252 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      253 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      254 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      255 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      256 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      257 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      258 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      259 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      260 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      261 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      262 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      263 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      264 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      265 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      266 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      267 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      268 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      269 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      270 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      271 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      272 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      273 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      274 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      275 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      276 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      277 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      278 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      279 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      280 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      281 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      282 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      283 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      284 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      285 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      286 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      287 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      288 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      289 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      290 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      291 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      292 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      293 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      294 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      295 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      296 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      297 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      298 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      299 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      300 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      301 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      302 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      303 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      304 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      305 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      306 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      307 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      308 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      309 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      310 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      311 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      312 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      313 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      314 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      315 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      316 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      317 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      318 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      319 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      320 data                2 AgeComp       NA <NA>       <NA>           NA    NA
          length datestart dateend  year  init distribution re_estimated log_like_cv
           <int> <chr>     <chr>   <int> <dbl> <chr>        <lgl>              <dbl>
        1     NA <NA>      <NA>       NA  162. Dlnorm       FALSE                 NA
        2     NA <NA>      <NA>       NA  461. Dlnorm       FALSE                 NA
        3     NA <NA>      <NA>       NA  747. Dlnorm       FALSE                 NA
        4     NA <NA>      <NA>       NA  997. Dlnorm       FALSE                 NA
        5     NA <NA>      <NA>       NA  768. Dlnorm       FALSE                 NA
        6     NA <NA>      <NA>       NA 1344. Dlnorm       FALSE                 NA
        7     NA <NA>      <NA>       NA 1319. Dlnorm       FALSE                 NA
        8     NA <NA>      <NA>       NA 2598. Dlnorm       FALSE                 NA
        9     NA <NA>      <NA>       NA 1426. Dlnorm       FALSE                 NA
       10     NA <NA>      <NA>       NA 1644. Dlnorm       FALSE                 NA
       11     NA <NA>      <NA>       NA 1771. Dlnorm       FALSE                 NA
       12     NA <NA>      <NA>       NA 1751. Dlnorm       FALSE                 NA
       13     NA <NA>      <NA>       NA 1194. Dlnorm       FALSE                 NA
       14     NA <NA>      <NA>       NA 1638. Dlnorm       FALSE                 NA
       15     NA <NA>      <NA>       NA 1584. Dlnorm       FALSE                 NA
       16     NA <NA>      <NA>       NA 1333. Dlnorm       FALSE                 NA
       17     NA <NA>      <NA>       NA 2325. Dlnorm       FALSE                 NA
       18     NA <NA>      <NA>       NA 1694. Dlnorm       FALSE                 NA
       19     NA <NA>      <NA>       NA 1532. Dlnorm       FALSE                 NA
       20     NA <NA>      <NA>       NA 1359. Dlnorm       FALSE                 NA
       21     NA <NA>      <NA>       NA 1639. Dlnorm       FALSE                 NA
       22     NA <NA>      <NA>       NA 1077. Dlnorm       FALSE                 NA
       23     NA <NA>      <NA>       NA 1648. Dlnorm       FALSE                 NA
       24     NA <NA>      <NA>       NA 1224. Dlnorm       FALSE                 NA
       25     NA <NA>      <NA>       NA 1127. Dlnorm       FALSE                 NA
       26     NA <NA>      <NA>       NA  956. Dlnorm       FALSE                 NA
       27     NA <NA>      <NA>       NA  902. Dlnorm       FALSE                 NA
       28     NA <NA>      <NA>       NA 1169. Dlnorm       FALSE                 NA
       29     NA <NA>      <NA>       NA  852. Dlnorm       FALSE                 NA
       30     NA <NA>      <NA>       NA 1274. Dlnorm       FALSE                 NA
       31     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
       32     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       33     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
       34     NA <NA>      <NA>       NA   30  Dmultinom    FALSE                 NA
       35     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       36     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
       37     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
       38     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       39     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
       40     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       41     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
       42     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
       43     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
       44     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
       45     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
       46     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       47     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
       48     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       49     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       50     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
       51     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       52     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
       53     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
       54     NA <NA>      <NA>       NA   27  Dmultinom    FALSE                 NA
       55     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       56     NA <NA>      <NA>       NA   30  Dmultinom    FALSE                 NA
       57     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
       58     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
       59     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
       60     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       61     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
       62     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
       63     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       64     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       65     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       66     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
       67     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
       68     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       69     NA <NA>      <NA>       NA   44  Dmultinom    FALSE                 NA
       70     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       71     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       72     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
       73     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       74     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       75     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       76     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       77     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
       78     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
       79     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
       80     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
       81     NA <NA>      <NA>       NA   32  Dmultinom    FALSE                 NA
       82     NA <NA>      <NA>       NA   34  Dmultinom    FALSE                 NA
       83     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       84     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
       85     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       86     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       87     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       88     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
       89     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       90     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
       91     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
       92     NA <NA>      <NA>       NA   37  Dmultinom    FALSE                 NA
       93     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
       94     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       95     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
       96     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
       97     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
       98     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       99     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      100     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      101     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      102     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      103     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      104     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
      105     NA <NA>      <NA>       NA   44  Dmultinom    FALSE                 NA
      106     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      107     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
      108     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      109     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      110     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      111     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      112     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      113     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      114     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      115     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      116     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      117     NA <NA>      <NA>       NA   44  Dmultinom    FALSE                 NA
      118     NA <NA>      <NA>       NA   40  Dmultinom    FALSE                 NA
      119     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      120     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      121     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      122     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      123     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      124     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      125     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      126     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      127     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      128     NA <NA>      <NA>       NA   32  Dmultinom    FALSE                 NA
      129     NA <NA>      <NA>       NA   27  Dmultinom    FALSE                 NA
      130     NA <NA>      <NA>       NA   35  Dmultinom    FALSE                 NA
      131     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
      132     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      133     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      134     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      135     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      136     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      137     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      138     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      139     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      140     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      141     NA <NA>      <NA>       NA   43  Dmultinom    FALSE                 NA
      142     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
      143     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      144     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      145     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      146     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      147     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      148     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      149     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      150     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      151     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      152     NA <NA>      <NA>       NA   34  Dmultinom    FALSE                 NA
      153     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      154     NA <NA>      <NA>       NA   43  Dmultinom    FALSE                 NA
      155     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      156     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      157     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      158     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      159     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      160     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      161     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      162     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      163     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      164     NA <NA>      <NA>       NA   32  Dmultinom    FALSE                 NA
      165     NA <NA>      <NA>       NA   30  Dmultinom    FALSE                 NA
      166     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
      167     NA <NA>      <NA>       NA   33  Dmultinom    FALSE                 NA
      168     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      169     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      170     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      171     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      172     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      173     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      174     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      175     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      176     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      177     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      178     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      179     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      180     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      181     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      182     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      183     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
      184     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      185     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      186     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      187     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      188     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      189     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      190     NA <NA>      <NA>       NA   41  Dmultinom    FALSE                 NA
      191     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      192     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      193     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      194     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      195     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      196     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      197     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      198     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      199     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      200     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      201     NA <NA>      <NA>       NA   39  Dmultinom    FALSE                 NA
      202     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
      203     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      204     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      205     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      206     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      207     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      208     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      209     NA <NA>      <NA>       NA    0  Dmultinom    FALSE                 NA
      210     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      211     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
      212     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      213     NA <NA>      <NA>       NA   39  Dmultinom    FALSE                 NA
      214     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      215     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      216     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      217     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      218     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      219     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      220     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      221     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      222     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      223     NA <NA>      <NA>       NA   33  Dmultinom    FALSE                 NA
      224     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      225     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      226     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
      227     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      228     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      229     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      230     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      231     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      232     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      233     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      234     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      235     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      236     NA <NA>      <NA>       NA   40  Dmultinom    FALSE                 NA
      237     NA <NA>      <NA>       NA   50  Dmultinom    FALSE                 NA
      238     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      239     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      240     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      241     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      242     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      243     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      244     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      245     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      246     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      247     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
      248     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      249     NA <NA>      <NA>       NA   37  Dmultinom    FALSE                 NA
      250     NA <NA>      <NA>       NA   34  Dmultinom    FALSE                 NA
      251     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      252     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      253     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      254     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      255     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      256     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      257     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      258     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      259     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      260     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
      261     NA <NA>      <NA>       NA   33  Dmultinom    FALSE                 NA
      262     NA <NA>      <NA>       NA   51  Dmultinom    FALSE                 NA
      263     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
      264     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      265     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      266     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      267     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      268     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      269     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      270     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      271     NA <NA>      <NA>       NA   49  Dmultinom    FALSE                 NA
      272     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      273     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      274     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      275     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      276     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      277     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      278     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      279     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      280     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      281     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      282     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      283     NA <NA>      <NA>       NA   40  Dmultinom    FALSE                 NA
      284     NA <NA>      <NA>       NA   67  Dmultinom    FALSE                 NA
      285     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      286     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      287     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      288     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      289     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      290     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      291     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      292     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      293     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      294     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      295     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      296     NA <NA>      <NA>       NA   51  Dmultinom    FALSE                 NA
      297     NA <NA>      <NA>       NA   56  Dmultinom    FALSE                 NA
      298     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
      299     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      300     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      301     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      302     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      303     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      304     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      305     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      306     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      307     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      308     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      309     NA <NA>      <NA>       NA   48  Dmultinom    FALSE                 NA
      310     NA <NA>      <NA>       NA   36  Dmultinom    FALSE                 NA
      311     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      312     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      313     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      314     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      315     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      316     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      317     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      318     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      319     NA <NA>      <NA>       NA   36  Dmultinom    FALSE                 NA
      320     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
          weight
           <dbl>
        1      1
        2      1
        3      1
        4      1
        5      1
        6      1
        7      1
        8      1
        9      1
       10      1
       11      1
       12      1
       13      1
       14      1
       15      1
       16      1
       17      1
       18      1
       19      1
       20      1
       21      1
       22      1
       23      1
       24      1
       25      1
       26      1
       27      1
       28      1
       29      1
       30      1
       31      1
       32      1
       33      1
       34      1
       35      1
       36      1
       37      1
       38      1
       39      1
       40      1
       41      1
       42      1
       43      1
       44      1
       45      1
       46      1
       47      1
       48      1
       49      1
       50      1
       51      1
       52      1
       53      1
       54      1
       55      1
       56      1
       57      1
       58      1
       59      1
       60      1
       61      1
       62      1
       63      1
       64      1
       65      1
       66      1
       67      1
       68      1
       69      1
       70      1
       71      1
       72      1
       73      1
       74      1
       75      1
       76      1
       77      1
       78      1
       79      1
       80      1
       81      1
       82      1
       83      1
       84      1
       85      1
       86      1
       87      1
       88      1
       89      1
       90      1
       91      1
       92      1
       93      1
       94      1
       95      1
       96      1
       97      1
       98      1
       99      1
      100      1
      101      1
      102      1
      103      1
      104      1
      105      1
      106      1
      107      1
      108      1
      109      1
      110      1
      111      1
      112      1
      113      1
      114      1
      115      1
      116      1
      117      1
      118      1
      119      1
      120      1
      121      1
      122      1
      123      1
      124      1
      125      1
      126      1
      127      1
      128      1
      129      1
      130      1
      131      1
      132      1
      133      1
      134      1
      135      1
      136      1
      137      1
      138      1
      139      1
      140      1
      141      1
      142      1
      143      1
      144      1
      145      1
      146      1
      147      1
      148      1
      149      1
      150      1
      151      1
      152      1
      153      1
      154      1
      155      1
      156      1
      157      1
      158      1
      159      1
      160      1
      161      1
      162      1
      163      1
      164      1
      165      1
      166      1
      167      1
      168      1
      169      1
      170      1
      171      1
      172      1
      173      1
      174      1
      175      1
      176      1
      177      1
      178      1
      179      1
      180      1
      181      1
      182      1
      183      1
      184      1
      185      1
      186      1
      187      1
      188      1
      189      1
      190      1
      191      1
      192      1
      193      1
      194      1
      195      1
      196      1
      197      1
      198      1
      199      1
      200      1
      201      1
      202      1
      203      1
      204      1
      205      1
      206      1
      207      1
      208      1
      209      1
      210      1
      211      1
      212      1
      213      1
      214      1
      215      1
      216      1
      217      1
      218      1
      219      1
      220      1
      221      1
      222      1
      223      1
      224      1
      225      1
      226      1
      227      1
      228      1
      229      1
      230      1
      231      1
      232      1
      233      1
      234      1
      235      1
      236      1
      237      1
      238      1
      239      1
      240      1
      241      1
      242      1
      243      1
      244      1
      245      1
      246      1
      247      1
      248      1
      249      1
      250      1
      251      1
      252      1
      253      1
      254      1
      255      1
      256      1
      257      1
      258      1
      259      1
      260      1
      261      1
      262      1
      263      1
      264      1
      265      1
      266      1
      267      1
      268      1
      269      1
      270      1
      271      1
      272      1
      273      1
      274      1
      275      1
      276      1
      277      1
      278      1
      279      1
      280      1
      281      1
      282      1
      283      1
      284      1
      285      1
      286      1
      287      1
      288      1
      289      1
      290      1
      291      1
      292      1
      293      1
      294      1
      295      1
      296      1
      297      1
      298      1
      299      1
      300      1
      301      1
      302      1
      303      1
      304      1
      305      1
      306      1
      307      1
      308      1
      309      1
      310      1
      311      1
      312      1
      313      1
      314      1
      315      1
      316      1
      317      1
      318      1
      319      1
      320      1
      # i 1,840 more rows

# get_fits() works with estimation run

    Code
      print(dplyr::select(get_fits(readRDS(fit_files[[1]])), -expected, -log_like),
      n = 320, width = Inf)
    Output
      # A tibble: 2,160 x 17
          module_name module_id label    data_id fleet_name unit  uncertainty   age
          <chr>           <int> <chr>      <int> <chr>      <chr>       <dbl> <int>
        1 data                1 Landings      NA <NA>       <NA>           NA    NA
        2 data                1 Landings      NA <NA>       <NA>           NA    NA
        3 data                1 Landings      NA <NA>       <NA>           NA    NA
        4 data                1 Landings      NA <NA>       <NA>           NA    NA
        5 data                1 Landings      NA <NA>       <NA>           NA    NA
        6 data                1 Landings      NA <NA>       <NA>           NA    NA
        7 data                1 Landings      NA <NA>       <NA>           NA    NA
        8 data                1 Landings      NA <NA>       <NA>           NA    NA
        9 data                1 Landings      NA <NA>       <NA>           NA    NA
       10 data                1 Landings      NA <NA>       <NA>           NA    NA
       11 data                1 Landings      NA <NA>       <NA>           NA    NA
       12 data                1 Landings      NA <NA>       <NA>           NA    NA
       13 data                1 Landings      NA <NA>       <NA>           NA    NA
       14 data                1 Landings      NA <NA>       <NA>           NA    NA
       15 data                1 Landings      NA <NA>       <NA>           NA    NA
       16 data                1 Landings      NA <NA>       <NA>           NA    NA
       17 data                1 Landings      NA <NA>       <NA>           NA    NA
       18 data                1 Landings      NA <NA>       <NA>           NA    NA
       19 data                1 Landings      NA <NA>       <NA>           NA    NA
       20 data                1 Landings      NA <NA>       <NA>           NA    NA
       21 data                1 Landings      NA <NA>       <NA>           NA    NA
       22 data                1 Landings      NA <NA>       <NA>           NA    NA
       23 data                1 Landings      NA <NA>       <NA>           NA    NA
       24 data                1 Landings      NA <NA>       <NA>           NA    NA
       25 data                1 Landings      NA <NA>       <NA>           NA    NA
       26 data                1 Landings      NA <NA>       <NA>           NA    NA
       27 data                1 Landings      NA <NA>       <NA>           NA    NA
       28 data                1 Landings      NA <NA>       <NA>           NA    NA
       29 data                1 Landings      NA <NA>       <NA>           NA    NA
       30 data                1 Landings      NA <NA>       <NA>           NA    NA
       31 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       32 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       33 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       34 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       35 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       36 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       37 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       38 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       39 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       40 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       41 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       42 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       43 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       44 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       45 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       46 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       47 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       48 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       49 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       50 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       51 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       52 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       53 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       54 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       55 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       56 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       57 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       58 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       59 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       60 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       61 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       62 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       63 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       64 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       65 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       66 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       67 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       68 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       69 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       70 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       71 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       72 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       73 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       74 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       75 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       76 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       77 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       78 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       79 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       80 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       81 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       82 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       83 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       84 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       85 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       86 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       87 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       88 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       89 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       90 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       91 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       92 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       93 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       94 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       95 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       96 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       97 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       98 data                2 AgeComp       NA <NA>       <NA>           NA    NA
       99 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      100 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      101 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      102 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      103 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      104 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      105 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      106 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      107 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      108 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      109 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      110 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      111 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      112 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      113 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      114 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      115 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      116 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      117 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      118 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      119 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      120 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      121 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      122 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      123 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      124 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      125 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      126 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      127 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      128 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      129 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      130 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      131 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      132 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      133 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      134 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      135 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      136 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      137 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      138 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      139 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      140 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      141 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      142 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      143 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      144 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      145 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      146 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      147 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      148 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      149 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      150 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      151 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      152 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      153 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      154 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      155 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      156 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      157 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      158 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      159 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      160 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      161 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      162 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      163 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      164 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      165 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      166 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      167 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      168 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      169 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      170 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      171 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      172 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      173 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      174 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      175 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      176 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      177 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      178 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      179 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      180 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      181 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      182 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      183 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      184 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      185 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      186 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      187 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      188 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      189 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      190 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      191 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      192 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      193 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      194 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      195 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      196 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      197 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      198 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      199 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      200 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      201 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      202 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      203 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      204 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      205 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      206 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      207 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      208 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      209 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      210 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      211 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      212 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      213 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      214 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      215 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      216 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      217 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      218 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      219 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      220 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      221 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      222 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      223 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      224 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      225 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      226 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      227 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      228 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      229 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      230 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      231 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      232 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      233 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      234 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      235 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      236 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      237 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      238 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      239 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      240 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      241 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      242 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      243 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      244 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      245 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      246 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      247 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      248 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      249 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      250 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      251 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      252 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      253 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      254 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      255 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      256 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      257 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      258 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      259 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      260 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      261 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      262 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      263 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      264 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      265 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      266 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      267 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      268 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      269 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      270 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      271 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      272 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      273 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      274 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      275 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      276 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      277 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      278 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      279 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      280 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      281 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      282 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      283 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      284 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      285 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      286 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      287 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      288 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      289 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      290 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      291 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      292 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      293 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      294 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      295 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      296 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      297 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      298 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      299 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      300 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      301 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      302 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      303 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      304 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      305 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      306 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      307 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      308 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      309 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      310 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      311 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      312 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      313 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      314 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      315 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      316 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      317 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      318 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      319 data                2 AgeComp       NA <NA>       <NA>           NA    NA
      320 data                2 AgeComp       NA <NA>       <NA>           NA    NA
          length datestart dateend  year  init distribution re_estimated log_like_cv
           <int> <chr>     <chr>   <int> <dbl> <chr>        <lgl>              <dbl>
        1     NA <NA>      <NA>       NA  162. Dlnorm       FALSE                 NA
        2     NA <NA>      <NA>       NA  461. Dlnorm       FALSE                 NA
        3     NA <NA>      <NA>       NA  747. Dlnorm       FALSE                 NA
        4     NA <NA>      <NA>       NA  997. Dlnorm       FALSE                 NA
        5     NA <NA>      <NA>       NA  768. Dlnorm       FALSE                 NA
        6     NA <NA>      <NA>       NA 1344. Dlnorm       FALSE                 NA
        7     NA <NA>      <NA>       NA 1319. Dlnorm       FALSE                 NA
        8     NA <NA>      <NA>       NA 2598. Dlnorm       FALSE                 NA
        9     NA <NA>      <NA>       NA 1426. Dlnorm       FALSE                 NA
       10     NA <NA>      <NA>       NA 1644. Dlnorm       FALSE                 NA
       11     NA <NA>      <NA>       NA 1771. Dlnorm       FALSE                 NA
       12     NA <NA>      <NA>       NA 1751. Dlnorm       FALSE                 NA
       13     NA <NA>      <NA>       NA 1194. Dlnorm       FALSE                 NA
       14     NA <NA>      <NA>       NA 1638. Dlnorm       FALSE                 NA
       15     NA <NA>      <NA>       NA 1584. Dlnorm       FALSE                 NA
       16     NA <NA>      <NA>       NA 1333. Dlnorm       FALSE                 NA
       17     NA <NA>      <NA>       NA 2325. Dlnorm       FALSE                 NA
       18     NA <NA>      <NA>       NA 1694. Dlnorm       FALSE                 NA
       19     NA <NA>      <NA>       NA 1532. Dlnorm       FALSE                 NA
       20     NA <NA>      <NA>       NA 1359. Dlnorm       FALSE                 NA
       21     NA <NA>      <NA>       NA 1639. Dlnorm       FALSE                 NA
       22     NA <NA>      <NA>       NA 1077. Dlnorm       FALSE                 NA
       23     NA <NA>      <NA>       NA 1648. Dlnorm       FALSE                 NA
       24     NA <NA>      <NA>       NA 1224. Dlnorm       FALSE                 NA
       25     NA <NA>      <NA>       NA 1127. Dlnorm       FALSE                 NA
       26     NA <NA>      <NA>       NA  956. Dlnorm       FALSE                 NA
       27     NA <NA>      <NA>       NA  902. Dlnorm       FALSE                 NA
       28     NA <NA>      <NA>       NA 1169. Dlnorm       FALSE                 NA
       29     NA <NA>      <NA>       NA  852. Dlnorm       FALSE                 NA
       30     NA <NA>      <NA>       NA 1274. Dlnorm       FALSE                 NA
       31     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
       32     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       33     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
       34     NA <NA>      <NA>       NA   30  Dmultinom    FALSE                 NA
       35     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       36     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
       37     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
       38     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       39     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
       40     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       41     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
       42     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
       43     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
       44     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
       45     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
       46     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       47     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
       48     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       49     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       50     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
       51     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       52     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
       53     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
       54     NA <NA>      <NA>       NA   27  Dmultinom    FALSE                 NA
       55     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       56     NA <NA>      <NA>       NA   30  Dmultinom    FALSE                 NA
       57     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
       58     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
       59     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
       60     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       61     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
       62     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
       63     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       64     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       65     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       66     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
       67     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
       68     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       69     NA <NA>      <NA>       NA   44  Dmultinom    FALSE                 NA
       70     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       71     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
       72     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
       73     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       74     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
       75     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       76     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       77     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
       78     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
       79     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
       80     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
       81     NA <NA>      <NA>       NA   32  Dmultinom    FALSE                 NA
       82     NA <NA>      <NA>       NA   34  Dmultinom    FALSE                 NA
       83     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       84     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
       85     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       86     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
       87     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       88     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
       89     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
       90     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
       91     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
       92     NA <NA>      <NA>       NA   37  Dmultinom    FALSE                 NA
       93     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
       94     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
       95     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
       96     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
       97     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
       98     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
       99     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      100     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      101     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      102     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      103     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      104     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
      105     NA <NA>      <NA>       NA   44  Dmultinom    FALSE                 NA
      106     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      107     NA <NA>      <NA>       NA   19  Dmultinom    FALSE                 NA
      108     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      109     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      110     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      111     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      112     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      113     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      114     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      115     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      116     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      117     NA <NA>      <NA>       NA   44  Dmultinom    FALSE                 NA
      118     NA <NA>      <NA>       NA   40  Dmultinom    FALSE                 NA
      119     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      120     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      121     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      122     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      123     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      124     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      125     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      126     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      127     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      128     NA <NA>      <NA>       NA   32  Dmultinom    FALSE                 NA
      129     NA <NA>      <NA>       NA   27  Dmultinom    FALSE                 NA
      130     NA <NA>      <NA>       NA   35  Dmultinom    FALSE                 NA
      131     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
      132     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      133     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      134     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      135     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      136     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      137     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      138     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      139     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      140     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      141     NA <NA>      <NA>       NA   43  Dmultinom    FALSE                 NA
      142     NA <NA>      <NA>       NA   20  Dmultinom    FALSE                 NA
      143     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      144     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      145     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      146     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      147     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      148     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      149     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      150     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      151     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      152     NA <NA>      <NA>       NA   34  Dmultinom    FALSE                 NA
      153     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      154     NA <NA>      <NA>       NA   43  Dmultinom    FALSE                 NA
      155     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      156     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      157     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      158     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      159     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      160     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      161     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      162     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      163     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      164     NA <NA>      <NA>       NA   32  Dmultinom    FALSE                 NA
      165     NA <NA>      <NA>       NA   30  Dmultinom    FALSE                 NA
      166     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
      167     NA <NA>      <NA>       NA   33  Dmultinom    FALSE                 NA
      168     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      169     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      170     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      171     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      172     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      173     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      174     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      175     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      176     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      177     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      178     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      179     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      180     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      181     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      182     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      183     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
      184     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      185     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      186     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      187     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      188     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      189     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      190     NA <NA>      <NA>       NA   41  Dmultinom    FALSE                 NA
      191     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      192     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      193     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      194     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      195     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      196     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      197     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      198     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      199     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      200     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      201     NA <NA>      <NA>       NA   39  Dmultinom    FALSE                 NA
      202     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
      203     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      204     NA <NA>      <NA>       NA   21  Dmultinom    FALSE                 NA
      205     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      206     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      207     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      208     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      209     NA <NA>      <NA>       NA    0  Dmultinom    FALSE                 NA
      210     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      211     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
      212     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      213     NA <NA>      <NA>       NA   39  Dmultinom    FALSE                 NA
      214     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      215     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      216     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      217     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      218     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      219     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      220     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      221     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      222     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      223     NA <NA>      <NA>       NA   33  Dmultinom    FALSE                 NA
      224     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      225     NA <NA>      <NA>       NA   28  Dmultinom    FALSE                 NA
      226     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
      227     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      228     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      229     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      230     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      231     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      232     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      233     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      234     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      235     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      236     NA <NA>      <NA>       NA   40  Dmultinom    FALSE                 NA
      237     NA <NA>      <NA>       NA   50  Dmultinom    FALSE                 NA
      238     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      239     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      240     NA <NA>      <NA>       NA   13  Dmultinom    FALSE                 NA
      241     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      242     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      243     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      244     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      245     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      246     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      247     NA <NA>      <NA>       NA   26  Dmultinom    FALSE                 NA
      248     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      249     NA <NA>      <NA>       NA   37  Dmultinom    FALSE                 NA
      250     NA <NA>      <NA>       NA   34  Dmultinom    FALSE                 NA
      251     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      252     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      253     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      254     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      255     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      256     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      257     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      258     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      259     NA <NA>      <NA>       NA    9  Dmultinom    FALSE                 NA
      260     NA <NA>      <NA>       NA   31  Dmultinom    FALSE                 NA
      261     NA <NA>      <NA>       NA   33  Dmultinom    FALSE                 NA
      262     NA <NA>      <NA>       NA   51  Dmultinom    FALSE                 NA
      263     NA <NA>      <NA>       NA   29  Dmultinom    FALSE                 NA
      264     NA <NA>      <NA>       NA   12  Dmultinom    FALSE                 NA
      265     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      266     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      267     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      268     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      269     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      270     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      271     NA <NA>      <NA>       NA   49  Dmultinom    FALSE                 NA
      272     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      273     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      274     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      275     NA <NA>      <NA>       NA   25  Dmultinom    FALSE                 NA
      276     NA <NA>      <NA>       NA   17  Dmultinom    FALSE                 NA
      277     NA <NA>      <NA>       NA   10  Dmultinom    FALSE                 NA
      278     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      279     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      280     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      281     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      282     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      283     NA <NA>      <NA>       NA   40  Dmultinom    FALSE                 NA
      284     NA <NA>      <NA>       NA   67  Dmultinom    FALSE                 NA
      285     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      286     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
      287     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      288     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      289     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      290     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      291     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      292     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      293     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      294     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      295     NA <NA>      <NA>       NA   22  Dmultinom    FALSE                 NA
      296     NA <NA>      <NA>       NA   51  Dmultinom    FALSE                 NA
      297     NA <NA>      <NA>       NA   56  Dmultinom    FALSE                 NA
      298     NA <NA>      <NA>       NA   16  Dmultinom    FALSE                 NA
      299     NA <NA>      <NA>       NA   11  Dmultinom    FALSE                 NA
      300     NA <NA>      <NA>       NA    7  Dmultinom    FALSE                 NA
      301     NA <NA>      <NA>       NA   18  Dmultinom    FALSE                 NA
      302     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      303     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      304     NA <NA>      <NA>       NA    3  Dmultinom    FALSE                 NA
      305     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      306     NA <NA>      <NA>       NA    5  Dmultinom    FALSE                 NA
      307     NA <NA>      <NA>       NA   24  Dmultinom    FALSE                 NA
      308     NA <NA>      <NA>       NA   38  Dmultinom    FALSE                 NA
      309     NA <NA>      <NA>       NA   48  Dmultinom    FALSE                 NA
      310     NA <NA>      <NA>       NA   36  Dmultinom    FALSE                 NA
      311     NA <NA>      <NA>       NA   14  Dmultinom    FALSE                 NA
      312     NA <NA>      <NA>       NA   15  Dmultinom    FALSE                 NA
      313     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      314     NA <NA>      <NA>       NA    6  Dmultinom    FALSE                 NA
      315     NA <NA>      <NA>       NA    8  Dmultinom    FALSE                 NA
      316     NA <NA>      <NA>       NA    1  Dmultinom    FALSE                 NA
      317     NA <NA>      <NA>       NA    4  Dmultinom    FALSE                 NA
      318     NA <NA>      <NA>       NA    2  Dmultinom    FALSE                 NA
      319     NA <NA>      <NA>       NA   36  Dmultinom    FALSE                 NA
      320     NA <NA>      <NA>       NA   23  Dmultinom    FALSE                 NA
          weight
           <dbl>
        1      1
        2      1
        3      1
        4      1
        5      1
        6      1
        7      1
        8      1
        9      1
       10      1
       11      1
       12      1
       13      1
       14      1
       15      1
       16      1
       17      1
       18      1
       19      1
       20      1
       21      1
       22      1
       23      1
       24      1
       25      1
       26      1
       27      1
       28      1
       29      1
       30      1
       31      1
       32      1
       33      1
       34      1
       35      1
       36      1
       37      1
       38      1
       39      1
       40      1
       41      1
       42      1
       43      1
       44      1
       45      1
       46      1
       47      1
       48      1
       49      1
       50      1
       51      1
       52      1
       53      1
       54      1
       55      1
       56      1
       57      1
       58      1
       59      1
       60      1
       61      1
       62      1
       63      1
       64      1
       65      1
       66      1
       67      1
       68      1
       69      1
       70      1
       71      1
       72      1
       73      1
       74      1
       75      1
       76      1
       77      1
       78      1
       79      1
       80      1
       81      1
       82      1
       83      1
       84      1
       85      1
       86      1
       87      1
       88      1
       89      1
       90      1
       91      1
       92      1
       93      1
       94      1
       95      1
       96      1
       97      1
       98      1
       99      1
      100      1
      101      1
      102      1
      103      1
      104      1
      105      1
      106      1
      107      1
      108      1
      109      1
      110      1
      111      1
      112      1
      113      1
      114      1
      115      1
      116      1
      117      1
      118      1
      119      1
      120      1
      121      1
      122      1
      123      1
      124      1
      125      1
      126      1
      127      1
      128      1
      129      1
      130      1
      131      1
      132      1
      133      1
      134      1
      135      1
      136      1
      137      1
      138      1
      139      1
      140      1
      141      1
      142      1
      143      1
      144      1
      145      1
      146      1
      147      1
      148      1
      149      1
      150      1
      151      1
      152      1
      153      1
      154      1
      155      1
      156      1
      157      1
      158      1
      159      1
      160      1
      161      1
      162      1
      163      1
      164      1
      165      1
      166      1
      167      1
      168      1
      169      1
      170      1
      171      1
      172      1
      173      1
      174      1
      175      1
      176      1
      177      1
      178      1
      179      1
      180      1
      181      1
      182      1
      183      1
      184      1
      185      1
      186      1
      187      1
      188      1
      189      1
      190      1
      191      1
      192      1
      193      1
      194      1
      195      1
      196      1
      197      1
      198      1
      199      1
      200      1
      201      1
      202      1
      203      1
      204      1
      205      1
      206      1
      207      1
      208      1
      209      1
      210      1
      211      1
      212      1
      213      1
      214      1
      215      1
      216      1
      217      1
      218      1
      219      1
      220      1
      221      1
      222      1
      223      1
      224      1
      225      1
      226      1
      227      1
      228      1
      229      1
      230      1
      231      1
      232      1
      233      1
      234      1
      235      1
      236      1
      237      1
      238      1
      239      1
      240      1
      241      1
      242      1
      243      1
      244      1
      245      1
      246      1
      247      1
      248      1
      249      1
      250      1
      251      1
      252      1
      253      1
      254      1
      255      1
      256      1
      257      1
      258      1
      259      1
      260      1
      261      1
      262      1
      263      1
      264      1
      265      1
      266      1
      267      1
      268      1
      269      1
      270      1
      271      1
      272      1
      273      1
      274      1
      275      1
      276      1
      277      1
      278      1
      279      1
      280      1
      281      1
      282      1
      283      1
      284      1
      285      1
      286      1
      287      1
      288      1
      289      1
      290      1
      291      1
      292      1
      293      1
      294      1
      295      1
      296      1
      297      1
      298      1
      299      1
      300      1
      301      1
      302      1
      303      1
      304      1
      305      1
      306      1
      307      1
      308      1
      309      1
      310      1
      311      1
      312      1
      313      1
      314      1
      315      1
      316      1
      317      1
      318      1
      319      1
      320      1
      # i 1,840 more rows

