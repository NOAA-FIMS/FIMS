# `get_estimates()` works with deterministic run

    Code
      estimates_snapshot <- dplyr::mutate(dplyr::select(get_estimates(
        deterministic_results), -estimated, -expected, -uncertainty, -gradient,
      -likelihood, -log_like_cv), dplyr::across(c(date, interval_end),
      observation_date_iso))
      estimates_snapshot_lines <- capture.output(suppressMessages(print(
        estimates_snapshot, n = 320, width = Inf)))
      cat(trimws(estimates_snapshot_lines, which = "right"), sep = "\n")
    Output
      # A tibble: 13,996 x 35
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
          parameter_id fleet  year_i sample_i age_i length_i  input observed
                 <int> <chr>   <int>    <int> <int>    <int>  <dbl>    <dbl>
        1            4 fleet1      1       NA    NA       NA -4.66        NA
        2           24 fleet1      2       NA    NA       NA -3.60        NA
        3           25 fleet1      3       NA    NA       NA -3.10        NA
        4           26 fleet1      4       NA    NA       NA -2.80        NA
        5           27 fleet1      5       NA    NA       NA -3.02        NA
        6           28 fleet1      6       NA    NA       NA -2.44        NA
        7           29 fleet1      7       NA    NA       NA -2.43        NA
        8           30 fleet1      8       NA    NA       NA -1.68        NA
        9           31 fleet1      9       NA    NA       NA -2.22        NA
       10           32 fleet1     10       NA    NA       NA -2.02        NA
       11           33 fleet1     11       NA    NA       NA -1.89        NA
       12           34 fleet1     12       NA    NA       NA -1.82        NA
       13           35 fleet1     13       NA    NA       NA -2.15        NA
       14           36 fleet1     14       NA    NA       NA -1.78        NA
       15           37 fleet1     15       NA    NA       NA -1.71        NA
       16           38 fleet1     16       NA    NA       NA -1.82        NA
       17           39 fleet1     17       NA    NA       NA -1.16        NA
       18           40 fleet1     18       NA    NA       NA -1.36        NA
       19           41 fleet1     19       NA    NA       NA -1.37        NA
       20           42 fleet1     20       NA    NA       NA -1.38        NA
       21           43 fleet1     21       NA    NA       NA -1.05        NA
       22           44 fleet1     22       NA    NA       NA -1.37        NA
       23           45 fleet1     23       NA    NA       NA -0.871       NA
       24           46 fleet1     24       NA    NA       NA -1.06        NA
       25           47 fleet1     25       NA    NA       NA -1.07        NA
       26           48 fleet1     26       NA    NA       NA -1.16        NA
       27           49 fleet1     27       NA    NA       NA -1.18        NA
       28           50 fleet1     28       NA    NA       NA -0.840       NA
       29           51 fleet1     29       NA    NA       NA -1.11        NA
       30           52 fleet1     30       NA    NA       NA -0.694       NA
       31            3 fleet1     NA       NA    NA       NA  0           NA
       32           NA fleet1      1        1     1       NA NA           14
       33           NA fleet1      1        1     2       NA NA           20
       34           NA fleet1      1        1     3       NA NA           23
       35           NA fleet1      1        1     4       NA NA           30
       36           NA fleet1      1        1     5       NA NA           20
       37           NA fleet1      1        1     6       NA NA           10
       38           NA fleet1      1        1     7       NA NA           22
       39           NA fleet1      1        1     8       NA NA           11
       40           NA fleet1      1        1     9       NA NA           12
       41           NA fleet1      1        1    10       NA NA            6
       42           NA fleet1      1        1    11       NA NA            7
       43           NA fleet1      1        1    12       NA NA           25
       44           NA fleet1      2        2     1       NA NA           21
       45           NA fleet1      2        2     2       NA NA           19
       46           NA fleet1      2        2     3       NA NA           22
       47           NA fleet1      2        2     4       NA NA           18
       48           NA fleet1      2        2     5       NA NA           31
       49           NA fleet1      2        2     6       NA NA           11
       50           NA fleet1      2        2     7       NA NA           13
       51           NA fleet1      2        2     8       NA NA           17
       52           NA fleet1      2        2     9       NA NA            9
       53           NA fleet1      2        2    10       NA NA            7
       54           NA fleet1      2        2    11       NA NA            5
       55           NA fleet1      2        2    12       NA NA           27
       56           NA fleet1      3        3     1       NA NA           11
       57           NA fleet1      3        3     2       NA NA           30
       58           NA fleet1      3        3     3       NA NA           21
       59           NA fleet1      3        3     4       NA NA           23
       60           NA fleet1      3        3     5       NA NA           19
       61           NA fleet1      3        3     6       NA NA           18
       62           NA fleet1      3        3     7       NA NA           14
       63           NA fleet1      3        3     8       NA NA           17
       64           NA fleet1      3        3     9       NA NA            6
       65           NA fleet1      3        3    10       NA NA            9
       66           NA fleet1      3        3    11       NA NA            6
       67           NA fleet1      3        3    12       NA NA           26
       68           NA fleet1      4        4     1       NA NA           10
       69           NA fleet1      4        4     2       NA NA           11
       70           NA fleet1      4        4     3       NA NA           44
       71           NA fleet1      4        4     4       NA NA           20
       72           NA fleet1      4        4     5       NA NA           20
       73           NA fleet1      4        4     6       NA NA           21
       74           NA fleet1      4        4     7       NA NA           13
       75           NA fleet1      4        4     8       NA NA           11
       76           NA fleet1      4        4     9       NA NA           13
       77           NA fleet1      4        4    10       NA NA            6
       78           NA fleet1      4        4    11       NA NA            3
       79           NA fleet1      4        4    12       NA NA           28
       80           NA fleet1      5        5     1       NA NA           31
       81           NA fleet1      5        5     2       NA NA           10
       82           NA fleet1      5        5     3       NA NA           32
       83           NA fleet1      5        5     4       NA NA           34
       84           NA fleet1      5        5     5       NA NA           18
       85           NA fleet1      5        5     6       NA NA           19
       86           NA fleet1      5        5     7       NA NA            9
       87           NA fleet1      5        5     8       NA NA            9
       88           NA fleet1      5        5     9       NA NA            6
       89           NA fleet1      5        5    10       NA NA            4
       90           NA fleet1      5        5    11       NA NA            6
       91           NA fleet1      5        5    12       NA NA           22
       92           NA fleet1      6        6     1       NA NA           16
       93           NA fleet1      6        6     2       NA NA           37
       94           NA fleet1      6        6     3       NA NA           14
       95           NA fleet1      6        6     4       NA NA           18
       96           NA fleet1      6        6     5       NA NA           29
       97           NA fleet1      6        6     6       NA NA           15
       98           NA fleet1      6        6     7       NA NA           12
       99           NA fleet1      6        6     8       NA NA           13
      100           NA fleet1      6        6     9       NA NA            8
      101           NA fleet1      6        6    10       NA NA            9
      102           NA fleet1      6        6    11       NA NA            5
      103           NA fleet1      6        6    12       NA NA           24
      104           NA fleet1      7        7     1       NA NA           11
      105           NA fleet1      7        7     2       NA NA           26
      106           NA fleet1      7        7     3       NA NA           44
      107           NA fleet1      7        7     4       NA NA           17
      108           NA fleet1      7        7     5       NA NA           19
      109           NA fleet1      7        7     6       NA NA           28
      110           NA fleet1      7        7     7       NA NA            7
      111           NA fleet1      7        7     8       NA NA            8
      112           NA fleet1      7        7     9       NA NA            7
      113           NA fleet1      7        7    10       NA NA            4
      114           NA fleet1      7        7    11       NA NA            5
      115           NA fleet1      7        7    12       NA NA           24
      116           NA fleet1      8        8     1       NA NA           18
      117           NA fleet1      8        8     2       NA NA           15
      118           NA fleet1      8        8     3       NA NA           44
      119           NA fleet1      8        8     4       NA NA           40
      120           NA fleet1      8        8     5       NA NA           12
      121           NA fleet1      8        8     6       NA NA            8
      122           NA fleet1      8        8     7       NA NA           14
      123           NA fleet1      8        8     8       NA NA           13
      124           NA fleet1      8        8     9       NA NA            5
      125           NA fleet1      8        8    10       NA NA            5
      126           NA fleet1      8        8    11       NA NA            5
      127           NA fleet1      8        8    12       NA NA           21
      128           NA fleet1      9        9     1       NA NA           14
      129           NA fleet1      9        9     2       NA NA           32
      130           NA fleet1      9        9     3       NA NA           27
      131           NA fleet1      9        9     4       NA NA           35
      132           NA fleet1      9        9     5       NA NA           31
      133           NA fleet1      9        9     6       NA NA            9
      134           NA fleet1      9        9     7       NA NA            9
      135           NA fleet1      9        9     8       NA NA           12
      136           NA fleet1      9        9     9       NA NA            5
      137           NA fleet1      9        9    10       NA NA            5
      138           NA fleet1      9        9    11       NA NA            3
      139           NA fleet1      9        9    12       NA NA           18
      140           NA fleet1     10       10     1       NA NA           15
      141           NA fleet1     10       10     2       NA NA           21
      142           NA fleet1     10       10     3       NA NA           43
      143           NA fleet1     10       10     4       NA NA           20
      144           NA fleet1     10       10     5       NA NA           22
      145           NA fleet1     10       10     6       NA NA           21
      146           NA fleet1     10       10     7       NA NA            7
      147           NA fleet1     10       10     8       NA NA            8
      148           NA fleet1     10       10     9       NA NA           13
      149           NA fleet1     10       10    10       NA NA            4
      150           NA fleet1     10       10    11       NA NA            4
      151           NA fleet1     10       10    12       NA NA           22
      152           NA fleet1     11       11     1       NA NA           15
      153           NA fleet1     11       11     2       NA NA           34
      154           NA fleet1     11       11     3       NA NA           23
      155           NA fleet1     11       11     4       NA NA           43
      156           NA fleet1     11       11     5       NA NA           15
      157           NA fleet1     11       11     6       NA NA           23
      158           NA fleet1     11       11     7       NA NA           21
      159           NA fleet1     11       11     8       NA NA            5
      160           NA fleet1     11       11     9       NA NA            3
      161           NA fleet1     11       11    10       NA NA            5
      162           NA fleet1     11       11    11       NA NA            2
      163           NA fleet1     11       11    12       NA NA           11
      164           NA fleet1     12       12     1       NA NA           11
      165           NA fleet1     12       12     2       NA NA           32
      166           NA fleet1     12       12     3       NA NA           30
      167           NA fleet1     12       12     4       NA NA           26
      168           NA fleet1     12       12     5       NA NA           33
      169           NA fleet1     12       12     6       NA NA           14
      170           NA fleet1     12       12     7       NA NA           13
      171           NA fleet1     12       12     8       NA NA           21
      172           NA fleet1     12       12     9       NA NA            4
      173           NA fleet1     12       12    10       NA NA            3
      174           NA fleet1     12       12    11       NA NA            4
      175           NA fleet1     12       12    12       NA NA            9
      176           NA fleet1     13       13     1       NA NA           22
      177           NA fleet1     13       13     2       NA NA           21
      178           NA fleet1     13       13     3       NA NA           23
      179           NA fleet1     13       13     4       NA NA           28
      180           NA fleet1     13       13     5       NA NA           23
      181           NA fleet1     13       13     6       NA NA           18
      182           NA fleet1     13       13     7       NA NA           15
      183           NA fleet1     13       13     8       NA NA           18
      184           NA fleet1     13       13     9       NA NA           16
      185           NA fleet1     13       13    10       NA NA            3
      186           NA fleet1     13       13    11       NA NA            3
      187           NA fleet1     13       13    12       NA NA           10
      188           NA fleet1     14       14     1       NA NA           23
      189           NA fleet1     14       14     2       NA NA           28
      190           NA fleet1     14       14     3       NA NA           17
      191           NA fleet1     14       14     4       NA NA           41
      192           NA fleet1     14       14     5       NA NA           24
      193           NA fleet1     14       14     6       NA NA           10
      194           NA fleet1     14       14     7       NA NA           10
      195           NA fleet1     14       14     8       NA NA            8
      196           NA fleet1     14       14     9       NA NA            9
      197           NA fleet1     14       14    10       NA NA           12
      198           NA fleet1     14       14    11       NA NA            5
      199           NA fleet1     14       14    12       NA NA           13
      200           NA fleet1     15       15     1       NA NA           14
      201           NA fleet1     15       15     2       NA NA           38
      202           NA fleet1     15       15     3       NA NA           39
      203           NA fleet1     15       15     4       NA NA           16
      204           NA fleet1     15       15     5       NA NA           28
      205           NA fleet1     15       15     6       NA NA           21
      206           NA fleet1     15       15     7       NA NA           10
      207           NA fleet1     15       15     8       NA NA           11
      208           NA fleet1     15       15     9       NA NA            5
      209           NA fleet1     15       15    10       NA NA            4
      210           NA fleet1     15       15    11       NA NA            0
      211           NA fleet1     15       15    12       NA NA           14
      212           NA fleet1     16       16     1       NA NA           29
      213           NA fleet1     16       16     2       NA NA           25
      214           NA fleet1     16       16     3       NA NA           39
      215           NA fleet1     16       16     4       NA NA           25
      216           NA fleet1     16       16     5       NA NA           17
      217           NA fleet1     16       16     6       NA NA           13
      218           NA fleet1     16       16     7       NA NA           12
      219           NA fleet1     16       16     8       NA NA            8
      220           NA fleet1     16       16     9       NA NA            7
      221           NA fleet1     16       16    10       NA NA            3
      222           NA fleet1     16       16    11       NA NA            7
      223           NA fleet1     16       16    12       NA NA           15
      224           NA fleet1     17       17     1       NA NA           33
      225           NA fleet1     17       17     2       NA NA           38
      226           NA fleet1     17       17     3       NA NA           28
      227           NA fleet1     17       17     4       NA NA           29
      228           NA fleet1     17       17     5       NA NA           24
      229           NA fleet1     17       17     6       NA NA            6
      230           NA fleet1     17       17     7       NA NA           10
      231           NA fleet1     17       17     8       NA NA            1
      232           NA fleet1     17       17     9       NA NA            6
      233           NA fleet1     17       17    10       NA NA            5
      234           NA fleet1     17       17    11       NA NA            2
      235           NA fleet1     17       17    12       NA NA           18
      236           NA fleet1     18       18     1       NA NA           17
      237           NA fleet1     18       18     2       NA NA           40
      238           NA fleet1     18       18     3       NA NA           50
      239           NA fleet1     18       18     4       NA NA           25
      240           NA fleet1     18       18     5       NA NA           17
      241           NA fleet1     18       18     6       NA NA           13
      242           NA fleet1     18       18     7       NA NA           11
      243           NA fleet1     18       18     8       NA NA            8
      244           NA fleet1     18       18     9       NA NA            7
      245           NA fleet1     18       18    10       NA NA            1
      246           NA fleet1     18       18    11       NA NA            4
      247           NA fleet1     18       18    12       NA NA            7
      248           NA fleet1     19       19     1       NA NA           26
      249           NA fleet1     19       19     2       NA NA           24
      250           NA fleet1     19       19     3       NA NA           37
      251           NA fleet1     19       19     4       NA NA           34
      252           NA fleet1     19       19     5       NA NA           22
      253           NA fleet1     19       19     6       NA NA           23
      254           NA fleet1     19       19     7       NA NA            8
      255           NA fleet1     19       19     8       NA NA            6
      256           NA fleet1     19       19     9       NA NA            3
      257           NA fleet1     19       19    10       NA NA            5
      258           NA fleet1     19       19    11       NA NA            4
      259           NA fleet1     19       19    12       NA NA            8
      260           NA fleet1     20       20     1       NA NA            9
      261           NA fleet1     20       20     2       NA NA           31
      262           NA fleet1     20       20     3       NA NA           33
      263           NA fleet1     20       20     4       NA NA           51
      264           NA fleet1     20       20     5       NA NA           29
      265           NA fleet1     20       20     6       NA NA           12
      266           NA fleet1     20       20     7       NA NA           11
      267           NA fleet1     20       20     8       NA NA           10
      268           NA fleet1     20       20     9       NA NA            5
      269           NA fleet1     20       20    10       NA NA            2
      270           NA fleet1     20       20    11       NA NA            5
      271           NA fleet1     20       20    12       NA NA            2
      272           NA fleet1     21       21     1       NA NA           49
      273           NA fleet1     21       21     2       NA NA           15
      274           NA fleet1     21       21     3       NA NA           38
      275           NA fleet1     21       21     4       NA NA           23
      276           NA fleet1     21       21     5       NA NA           25
      277           NA fleet1     21       21     6       NA NA           17
      278           NA fleet1     21       21     7       NA NA           10
      279           NA fleet1     21       21     8       NA NA            8
      280           NA fleet1     21       21     9       NA NA            2
      281           NA fleet1     21       21    10       NA NA            1
      282           NA fleet1     21       21    11       NA NA            5
      283           NA fleet1     21       21    12       NA NA            7
      284           NA fleet1     22       22     1       NA NA           40
      285           NA fleet1     22       22     2       NA NA           67
      286           NA fleet1     22       22     3       NA NA           14
      287           NA fleet1     22       22     4       NA NA           23
      288           NA fleet1     22       22     5       NA NA           15
      289           NA fleet1     22       22     6       NA NA            6
      290           NA fleet1     22       22     7       NA NA           14
      291           NA fleet1     22       22     8       NA NA            4
      292           NA fleet1     22       22     9       NA NA            5
      293           NA fleet1     22       22    10       NA NA            4
      294           NA fleet1     22       22    11       NA NA            1
      295           NA fleet1     22       22    12       NA NA            7
      296           NA fleet1     23       23     1       NA NA           22
      297           NA fleet1     23       23     2       NA NA           51
      298           NA fleet1     23       23     3       NA NA           56
      299           NA fleet1     23       23     4       NA NA           16
      300           NA fleet1     23       23     5       NA NA           11
      301           NA fleet1     23       23     6       NA NA            7
      302           NA fleet1     23       23     7       NA NA           18
      303           NA fleet1     23       23     8       NA NA            5
      304           NA fleet1     23       23     9       NA NA            1
      305           NA fleet1     23       23    10       NA NA            3
      306           NA fleet1     23       23    11       NA NA            5
      307           NA fleet1     23       23    12       NA NA            5
      308           NA fleet1     24       24     1       NA NA           24
      309           NA fleet1     24       24     2       NA NA           38
      310           NA fleet1     24       24     3       NA NA           48
      311           NA fleet1     24       24     4       NA NA           36
      312           NA fleet1     24       24     5       NA NA           14
      313           NA fleet1     24       24     6       NA NA           15
      314           NA fleet1     24       24     7       NA NA            4
      315           NA fleet1     24       24     8       NA NA            6
      316           NA fleet1     24       24     9       NA NA            8
      317           NA fleet1     24       24    10       NA NA            1
      318           NA fleet1     24       24    11       NA NA            4
      319           NA fleet1     24       24    12       NA NA            2
      320           NA fleet1     25       25     1       NA NA           36
          estimation_type  distribution input_type  lpdf log_sd timing date
          <chr>            <chr>        <chr>      <dbl>  <dbl>  <int> <chr>
        1 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        2 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        3 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        4 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        5 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        6 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        7 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        8 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        9 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       10 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       11 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       12 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       13 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       14 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       15 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       16 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       17 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       18 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       19 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       20 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       21 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       22 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       23 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       24 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       25 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       26 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       27 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       28 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       29 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       30 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       31 constant         <NA>         <NA>         NA      NA     NA <NA>
       32 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       33 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       34 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       35 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       36 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       37 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       38 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       39 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       40 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       41 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       42 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       43 derived_quantity multinomial  data       -836.     NA      1 0001-01-01
       44 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       45 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       46 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       47 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       48 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       49 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       50 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       51 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       52 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       53 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       54 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       55 derived_quantity multinomial  data       -836.     NA      2 0002-01-01
       56 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       57 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       58 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       59 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       60 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       61 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       62 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       63 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       64 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       65 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       66 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       67 derived_quantity multinomial  data       -836.     NA      3 0003-01-01
       68 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       69 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       70 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       71 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       72 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       73 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       74 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       75 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       76 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       77 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       78 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       79 derived_quantity multinomial  data       -836.     NA      4 0004-01-01
       80 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       81 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       82 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       83 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       84 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       85 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       86 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       87 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       88 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       89 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       90 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       91 derived_quantity multinomial  data       -836.     NA      5 0005-01-01
       92 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       93 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       94 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       95 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       96 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       97 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       98 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
       99 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
      100 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
      101 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
      102 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
      103 derived_quantity multinomial  data       -836.     NA      6 0006-01-01
      104 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      105 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      106 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      107 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      108 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      109 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      110 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      111 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      112 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      113 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      114 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      115 derived_quantity multinomial  data       -836.     NA      7 0007-01-01
      116 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      117 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      118 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      119 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      120 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      121 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      122 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      123 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      124 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      125 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      126 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      127 derived_quantity multinomial  data       -836.     NA      8 0008-01-01
      128 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      129 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      130 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      131 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      132 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      133 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      134 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      135 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      136 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      137 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      138 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      139 derived_quantity multinomial  data       -836.     NA      9 0009-01-01
      140 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      141 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      142 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      143 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      144 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      145 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      146 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      147 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      148 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      149 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      150 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      151 derived_quantity multinomial  data       -836.     NA     10 0010-01-01
      152 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      153 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      154 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      155 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      156 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      157 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      158 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      159 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      160 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      161 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      162 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      163 derived_quantity multinomial  data       -836.     NA     11 0011-01-01
      164 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      165 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      166 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      167 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      168 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      169 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      170 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      171 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      172 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      173 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      174 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      175 derived_quantity multinomial  data       -836.     NA     12 0012-01-01
      176 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      177 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      178 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      179 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      180 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      181 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      182 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      183 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      184 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      185 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      186 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      187 derived_quantity multinomial  data       -836.     NA     13 0013-01-01
      188 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      189 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      190 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      191 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      192 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      193 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      194 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      195 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      196 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      197 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      198 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      199 derived_quantity multinomial  data       -836.     NA     14 0014-01-01
      200 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      201 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      202 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      203 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      204 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      205 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      206 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      207 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      208 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      209 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      210 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      211 derived_quantity multinomial  data       -836.     NA     15 0015-01-01
      212 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      213 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      214 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      215 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      216 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      217 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      218 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      219 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      220 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      221 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      222 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      223 derived_quantity multinomial  data       -836.     NA     16 0016-01-01
      224 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      225 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      226 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      227 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      228 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      229 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      230 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      231 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      232 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      233 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      234 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      235 derived_quantity multinomial  data       -836.     NA     17 0017-01-01
      236 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      237 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      238 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      239 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      240 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      241 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      242 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      243 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      244 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      245 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      246 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      247 derived_quantity multinomial  data       -836.     NA     18 0018-01-01
      248 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      249 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      250 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      251 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      252 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      253 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      254 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      255 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      256 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      257 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      258 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      259 derived_quantity multinomial  data       -836.     NA     19 0019-01-01
      260 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      261 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      262 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      263 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      264 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      265 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      266 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      267 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      268 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      269 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      270 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      271 derived_quantity multinomial  data       -836.     NA     20 0020-01-01
      272 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      273 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      274 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      275 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      276 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      277 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      278 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      279 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      280 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      281 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      282 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      283 derived_quantity multinomial  data       -836.     NA     21 0021-01-01
      284 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      285 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      286 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      287 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      288 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      289 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      290 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      291 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      292 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      293 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      294 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      295 derived_quantity multinomial  data       -836.     NA     22 0022-01-01
      296 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      297 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      298 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      299 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      300 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      301 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      302 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      303 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      304 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      305 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      306 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      307 derived_quantity multinomial  data       -836.     NA     23 0023-01-01
      308 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      309 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      310 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      311 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      312 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      313 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      314 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      315 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      316 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      317 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      318 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      319 derived_quantity multinomial  data       -836.     NA     24 0024-01-01
      320 derived_quantity multinomial  data       -836.     NA     25 0025-01-01
          input_precision prediction_basis support  interval_end
          <chr>           <chr>            <chr>    <chr>
        1 <NA>            <NA>             <NA>     <NA>
        2 <NA>            <NA>             <NA>     <NA>
        3 <NA>            <NA>             <NA>     <NA>
        4 <NA>            <NA>             <NA>     <NA>
        5 <NA>            <NA>             <NA>     <NA>
        6 <NA>            <NA>             <NA>     <NA>
        7 <NA>            <NA>             <NA>     <NA>
        8 <NA>            <NA>             <NA>     <NA>
        9 <NA>            <NA>             <NA>     <NA>
       10 <NA>            <NA>             <NA>     <NA>
       11 <NA>            <NA>             <NA>     <NA>
       12 <NA>            <NA>             <NA>     <NA>
       13 <NA>            <NA>             <NA>     <NA>
       14 <NA>            <NA>             <NA>     <NA>
       15 <NA>            <NA>             <NA>     <NA>
       16 <NA>            <NA>             <NA>     <NA>
       17 <NA>            <NA>             <NA>     <NA>
       18 <NA>            <NA>             <NA>     <NA>
       19 <NA>            <NA>             <NA>     <NA>
       20 <NA>            <NA>             <NA>     <NA>
       21 <NA>            <NA>             <NA>     <NA>
       22 <NA>            <NA>             <NA>     <NA>
       23 <NA>            <NA>             <NA>     <NA>
       24 <NA>            <NA>             <NA>     <NA>
       25 <NA>            <NA>             <NA>     <NA>
       26 <NA>            <NA>             <NA>     <NA>
       27 <NA>            <NA>             <NA>     <NA>
       28 <NA>            <NA>             <NA>     <NA>
       29 <NA>            <NA>             <NA>     <NA>
       30 <NA>            <NA>             <NA>     <NA>
       31 <NA>            <NA>             <NA>     <NA>
       32 year            catch            interval 0002-01-01
       33 year            catch            interval 0002-01-01
       34 year            catch            interval 0002-01-01
       35 year            catch            interval 0002-01-01
       36 year            catch            interval 0002-01-01
       37 year            catch            interval 0002-01-01
       38 year            catch            interval 0002-01-01
       39 year            catch            interval 0002-01-01
       40 year            catch            interval 0002-01-01
       41 year            catch            interval 0002-01-01
       42 year            catch            interval 0002-01-01
       43 year            catch            interval 0002-01-01
       44 year            catch            interval 0003-01-01
       45 year            catch            interval 0003-01-01
       46 year            catch            interval 0003-01-01
       47 year            catch            interval 0003-01-01
       48 year            catch            interval 0003-01-01
       49 year            catch            interval 0003-01-01
       50 year            catch            interval 0003-01-01
       51 year            catch            interval 0003-01-01
       52 year            catch            interval 0003-01-01
       53 year            catch            interval 0003-01-01
       54 year            catch            interval 0003-01-01
       55 year            catch            interval 0003-01-01
       56 year            catch            interval 0004-01-01
       57 year            catch            interval 0004-01-01
       58 year            catch            interval 0004-01-01
       59 year            catch            interval 0004-01-01
       60 year            catch            interval 0004-01-01
       61 year            catch            interval 0004-01-01
       62 year            catch            interval 0004-01-01
       63 year            catch            interval 0004-01-01
       64 year            catch            interval 0004-01-01
       65 year            catch            interval 0004-01-01
       66 year            catch            interval 0004-01-01
       67 year            catch            interval 0004-01-01
       68 year            catch            interval 0005-01-01
       69 year            catch            interval 0005-01-01
       70 year            catch            interval 0005-01-01
       71 year            catch            interval 0005-01-01
       72 year            catch            interval 0005-01-01
       73 year            catch            interval 0005-01-01
       74 year            catch            interval 0005-01-01
       75 year            catch            interval 0005-01-01
       76 year            catch            interval 0005-01-01
       77 year            catch            interval 0005-01-01
       78 year            catch            interval 0005-01-01
       79 year            catch            interval 0005-01-01
       80 year            catch            interval 0006-01-01
       81 year            catch            interval 0006-01-01
       82 year            catch            interval 0006-01-01
       83 year            catch            interval 0006-01-01
       84 year            catch            interval 0006-01-01
       85 year            catch            interval 0006-01-01
       86 year            catch            interval 0006-01-01
       87 year            catch            interval 0006-01-01
       88 year            catch            interval 0006-01-01
       89 year            catch            interval 0006-01-01
       90 year            catch            interval 0006-01-01
       91 year            catch            interval 0006-01-01
       92 year            catch            interval 0007-01-01
       93 year            catch            interval 0007-01-01
       94 year            catch            interval 0007-01-01
       95 year            catch            interval 0007-01-01
       96 year            catch            interval 0007-01-01
       97 year            catch            interval 0007-01-01
       98 year            catch            interval 0007-01-01
       99 year            catch            interval 0007-01-01
      100 year            catch            interval 0007-01-01
      101 year            catch            interval 0007-01-01
      102 year            catch            interval 0007-01-01
      103 year            catch            interval 0007-01-01
      104 year            catch            interval 0008-01-01
      105 year            catch            interval 0008-01-01
      106 year            catch            interval 0008-01-01
      107 year            catch            interval 0008-01-01
      108 year            catch            interval 0008-01-01
      109 year            catch            interval 0008-01-01
      110 year            catch            interval 0008-01-01
      111 year            catch            interval 0008-01-01
      112 year            catch            interval 0008-01-01
      113 year            catch            interval 0008-01-01
      114 year            catch            interval 0008-01-01
      115 year            catch            interval 0008-01-01
      116 year            catch            interval 0009-01-01
      117 year            catch            interval 0009-01-01
      118 year            catch            interval 0009-01-01
      119 year            catch            interval 0009-01-01
      120 year            catch            interval 0009-01-01
      121 year            catch            interval 0009-01-01
      122 year            catch            interval 0009-01-01
      123 year            catch            interval 0009-01-01
      124 year            catch            interval 0009-01-01
      125 year            catch            interval 0009-01-01
      126 year            catch            interval 0009-01-01
      127 year            catch            interval 0009-01-01
      128 year            catch            interval 0010-01-01
      129 year            catch            interval 0010-01-01
      130 year            catch            interval 0010-01-01
      131 year            catch            interval 0010-01-01
      132 year            catch            interval 0010-01-01
      133 year            catch            interval 0010-01-01
      134 year            catch            interval 0010-01-01
      135 year            catch            interval 0010-01-01
      136 year            catch            interval 0010-01-01
      137 year            catch            interval 0010-01-01
      138 year            catch            interval 0010-01-01
      139 year            catch            interval 0010-01-01
      140 year            catch            interval 0011-01-01
      141 year            catch            interval 0011-01-01
      142 year            catch            interval 0011-01-01
      143 year            catch            interval 0011-01-01
      144 year            catch            interval 0011-01-01
      145 year            catch            interval 0011-01-01
      146 year            catch            interval 0011-01-01
      147 year            catch            interval 0011-01-01
      148 year            catch            interval 0011-01-01
      149 year            catch            interval 0011-01-01
      150 year            catch            interval 0011-01-01
      151 year            catch            interval 0011-01-01
      152 year            catch            interval 0012-01-01
      153 year            catch            interval 0012-01-01
      154 year            catch            interval 0012-01-01
      155 year            catch            interval 0012-01-01
      156 year            catch            interval 0012-01-01
      157 year            catch            interval 0012-01-01
      158 year            catch            interval 0012-01-01
      159 year            catch            interval 0012-01-01
      160 year            catch            interval 0012-01-01
      161 year            catch            interval 0012-01-01
      162 year            catch            interval 0012-01-01
      163 year            catch            interval 0012-01-01
      164 year            catch            interval 0013-01-01
      165 year            catch            interval 0013-01-01
      166 year            catch            interval 0013-01-01
      167 year            catch            interval 0013-01-01
      168 year            catch            interval 0013-01-01
      169 year            catch            interval 0013-01-01
      170 year            catch            interval 0013-01-01
      171 year            catch            interval 0013-01-01
      172 year            catch            interval 0013-01-01
      173 year            catch            interval 0013-01-01
      174 year            catch            interval 0013-01-01
      175 year            catch            interval 0013-01-01
      176 year            catch            interval 0014-01-01
      177 year            catch            interval 0014-01-01
      178 year            catch            interval 0014-01-01
      179 year            catch            interval 0014-01-01
      180 year            catch            interval 0014-01-01
      181 year            catch            interval 0014-01-01
      182 year            catch            interval 0014-01-01
      183 year            catch            interval 0014-01-01
      184 year            catch            interval 0014-01-01
      185 year            catch            interval 0014-01-01
      186 year            catch            interval 0014-01-01
      187 year            catch            interval 0014-01-01
      188 year            catch            interval 0015-01-01
      189 year            catch            interval 0015-01-01
      190 year            catch            interval 0015-01-01
      191 year            catch            interval 0015-01-01
      192 year            catch            interval 0015-01-01
      193 year            catch            interval 0015-01-01
      194 year            catch            interval 0015-01-01
      195 year            catch            interval 0015-01-01
      196 year            catch            interval 0015-01-01
      197 year            catch            interval 0015-01-01
      198 year            catch            interval 0015-01-01
      199 year            catch            interval 0015-01-01
      200 year            catch            interval 0016-01-01
      201 year            catch            interval 0016-01-01
      202 year            catch            interval 0016-01-01
      203 year            catch            interval 0016-01-01
      204 year            catch            interval 0016-01-01
      205 year            catch            interval 0016-01-01
      206 year            catch            interval 0016-01-01
      207 year            catch            interval 0016-01-01
      208 year            catch            interval 0016-01-01
      209 year            catch            interval 0016-01-01
      210 year            catch            interval 0016-01-01
      211 year            catch            interval 0016-01-01
      212 year            catch            interval 0017-01-01
      213 year            catch            interval 0017-01-01
      214 year            catch            interval 0017-01-01
      215 year            catch            interval 0017-01-01
      216 year            catch            interval 0017-01-01
      217 year            catch            interval 0017-01-01
      218 year            catch            interval 0017-01-01
      219 year            catch            interval 0017-01-01
      220 year            catch            interval 0017-01-01
      221 year            catch            interval 0017-01-01
      222 year            catch            interval 0017-01-01
      223 year            catch            interval 0017-01-01
      224 year            catch            interval 0018-01-01
      225 year            catch            interval 0018-01-01
      226 year            catch            interval 0018-01-01
      227 year            catch            interval 0018-01-01
      228 year            catch            interval 0018-01-01
      229 year            catch            interval 0018-01-01
      230 year            catch            interval 0018-01-01
      231 year            catch            interval 0018-01-01
      232 year            catch            interval 0018-01-01
      233 year            catch            interval 0018-01-01
      234 year            catch            interval 0018-01-01
      235 year            catch            interval 0018-01-01
      236 year            catch            interval 0019-01-01
      237 year            catch            interval 0019-01-01
      238 year            catch            interval 0019-01-01
      239 year            catch            interval 0019-01-01
      240 year            catch            interval 0019-01-01
      241 year            catch            interval 0019-01-01
      242 year            catch            interval 0019-01-01
      243 year            catch            interval 0019-01-01
      244 year            catch            interval 0019-01-01
      245 year            catch            interval 0019-01-01
      246 year            catch            interval 0019-01-01
      247 year            catch            interval 0019-01-01
      248 year            catch            interval 0020-01-01
      249 year            catch            interval 0020-01-01
      250 year            catch            interval 0020-01-01
      251 year            catch            interval 0020-01-01
      252 year            catch            interval 0020-01-01
      253 year            catch            interval 0020-01-01
      254 year            catch            interval 0020-01-01
      255 year            catch            interval 0020-01-01
      256 year            catch            interval 0020-01-01
      257 year            catch            interval 0020-01-01
      258 year            catch            interval 0020-01-01
      259 year            catch            interval 0020-01-01
      260 year            catch            interval 0021-01-01
      261 year            catch            interval 0021-01-01
      262 year            catch            interval 0021-01-01
      263 year            catch            interval 0021-01-01
      264 year            catch            interval 0021-01-01
      265 year            catch            interval 0021-01-01
      266 year            catch            interval 0021-01-01
      267 year            catch            interval 0021-01-01
      268 year            catch            interval 0021-01-01
      269 year            catch            interval 0021-01-01
      270 year            catch            interval 0021-01-01
      271 year            catch            interval 0021-01-01
      272 year            catch            interval 0022-01-01
      273 year            catch            interval 0022-01-01
      274 year            catch            interval 0022-01-01
      275 year            catch            interval 0022-01-01
      276 year            catch            interval 0022-01-01
      277 year            catch            interval 0022-01-01
      278 year            catch            interval 0022-01-01
      279 year            catch            interval 0022-01-01
      280 year            catch            interval 0022-01-01
      281 year            catch            interval 0022-01-01
      282 year            catch            interval 0022-01-01
      283 year            catch            interval 0022-01-01
      284 year            catch            interval 0023-01-01
      285 year            catch            interval 0023-01-01
      286 year            catch            interval 0023-01-01
      287 year            catch            interval 0023-01-01
      288 year            catch            interval 0023-01-01
      289 year            catch            interval 0023-01-01
      290 year            catch            interval 0023-01-01
      291 year            catch            interval 0023-01-01
      292 year            catch            interval 0023-01-01
      293 year            catch            interval 0023-01-01
      294 year            catch            interval 0023-01-01
      295 year            catch            interval 0023-01-01
      296 year            catch            interval 0024-01-01
      297 year            catch            interval 0024-01-01
      298 year            catch            interval 0024-01-01
      299 year            catch            interval 0024-01-01
      300 year            catch            interval 0024-01-01
      301 year            catch            interval 0024-01-01
      302 year            catch            interval 0024-01-01
      303 year            catch            interval 0024-01-01
      304 year            catch            interval 0024-01-01
      305 year            catch            interval 0024-01-01
      306 year            catch            interval 0024-01-01
      307 year            catch            interval 0024-01-01
      308 year            catch            interval 0025-01-01
      309 year            catch            interval 0025-01-01
      310 year            catch            interval 0025-01-01
      311 year            catch            interval 0025-01-01
      312 year            catch            interval 0025-01-01
      313 year            catch            interval 0025-01-01
      314 year            catch            interval 0025-01-01
      315 year            catch            interval 0025-01-01
      316 year            catch            interval 0025-01-01
      317 year            catch            interval 0025-01-01
      318 year            catch            interval 0025-01-01
      319 year            catch            interval 0025-01-01
      320 year            catch            interval 0026-01-01
          observation_id                            partition population
          <chr>                                     <chr>     <chr>
        1 <NA>                                      <NA>      <NA>
        2 <NA>                                      <NA>      <NA>
        3 <NA>                                      <NA>      <NA>
        4 <NA>                                      <NA>      <NA>
        5 <NA>                                      <NA>      <NA>
        6 <NA>                                      <NA>      <NA>
        7 <NA>                                      <NA>      <NA>
        8 <NA>                                      <NA>      <NA>
        9 <NA>                                      <NA>      <NA>
       10 <NA>                                      <NA>      <NA>
       11 <NA>                                      <NA>      <NA>
       12 <NA>                                      <NA>      <NA>
       13 <NA>                                      <NA>      <NA>
       14 <NA>                                      <NA>      <NA>
       15 <NA>                                      <NA>      <NA>
       16 <NA>                                      <NA>      <NA>
       17 <NA>                                      <NA>      <NA>
       18 <NA>                                      <NA>      <NA>
       19 <NA>                                      <NA>      <NA>
       20 <NA>                                      <NA>      <NA>
       21 <NA>                                      <NA>      <NA>
       22 <NA>                                      <NA>      <NA>
       23 <NA>                                      <NA>      <NA>
       24 <NA>                                      <NA>      <NA>
       25 <NA>                                      <NA>      <NA>
       26 <NA>                                      <NA>      <NA>
       27 <NA>                                      <NA>      <NA>
       28 <NA>                                      <NA>      <NA>
       29 <NA>                                      <NA>      <NA>
       30 <NA>                                      <NA>      <NA>
       31 <NA>                                      <NA>      <NA>
       32 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       33 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       34 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       35 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       36 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       37 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       38 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       39 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       40 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       41 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       42 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       43 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       44 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       45 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       46 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       47 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       48 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       49 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       50 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       51 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       52 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       53 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       54 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       55 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       56 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       57 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       58 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       59 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       60 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       61 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       62 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       63 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       64 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       65 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       66 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       67 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       68 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       69 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       70 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       71 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       72 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       73 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       74 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       75 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       76 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       77 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       78 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       79 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       80 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       81 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       82 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       83 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       84 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       85 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       86 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       87 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       88 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       89 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       90 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       91 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       92 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       93 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       94 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       95 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       96 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       97 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       98 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       99 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      100 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      101 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      102 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      103 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      104 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      105 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      106 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      107 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      108 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      109 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      110 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      111 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      112 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      113 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      114 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      115 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      116 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      117 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      118 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      119 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      120 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      121 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      122 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      123 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      124 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      125 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      126 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      127 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      128 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      129 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      130 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      131 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      132 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      133 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      134 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      135 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      136 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      137 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      138 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      139 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      140 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      141 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      142 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      143 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      144 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      145 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      146 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      147 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      148 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      149 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      150 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      151 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      152 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      153 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      154 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      155 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      156 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      157 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      158 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      159 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      160 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      161 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      162 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      163 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      164 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      165 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      166 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      167 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      168 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      169 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      170 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      171 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      172 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      173 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      174 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      175 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      176 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      177 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      178 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      179 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      180 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      181 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      182 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      183 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      184 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      185 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      186 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      187 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      188 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      189 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      190 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      191 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      192 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      193 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      194 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      195 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      196 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      197 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      198 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      199 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      200 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      201 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      202 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      203 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      204 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      205 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      206 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      207 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      208 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      209 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      210 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      211 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      212 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      213 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      214 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      215 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      216 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      217 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      218 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      219 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      220 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      221 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      222 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      223 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      224 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      225 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      226 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      227 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      228 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      229 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      230 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      231 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      232 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      233 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      234 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      235 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      236 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      237 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      238 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      239 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      240 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      241 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      242 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      243 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      244 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      245 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      246 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      247 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      248 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      249 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      250 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      251 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      252 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      253 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      254 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      255 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      256 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      257 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      258 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      259 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      260 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      261 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      262 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      263 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      264 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      265 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      266 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      267 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      268 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      269 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      270 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      271 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      272 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      273 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      274 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      275 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      276 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      277 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      278 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      279 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      280 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      281 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      282 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      283 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      284 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      285 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      286 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      287 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      288 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      289 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      290 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      291 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      292 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      293 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      294 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      295 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      296 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      297 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      298 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      299 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      300 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      301 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      302 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      303 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      304 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      305 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      306 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      307 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      308 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      309 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      310 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      311 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      312 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      313 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      314 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      315 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      316 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      317 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      318 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      319 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      320 6:fleet1|8:age_comp|10:0025-01-01|5:catch pooled    population1
          year_fraction   day time_id weight_timing length_mapping_timing
                  <dbl> <int>   <int> <chr>         <chr>
        1            NA    NA      NA <NA>          <NA>
        2            NA    NA      NA <NA>          <NA>
        3            NA    NA      NA <NA>          <NA>
        4            NA    NA      NA <NA>          <NA>
        5            NA    NA      NA <NA>          <NA>
        6            NA    NA      NA <NA>          <NA>
        7            NA    NA      NA <NA>          <NA>
        8            NA    NA      NA <NA>          <NA>
        9            NA    NA      NA <NA>          <NA>
       10            NA    NA      NA <NA>          <NA>
       11            NA    NA      NA <NA>          <NA>
       12            NA    NA      NA <NA>          <NA>
       13            NA    NA      NA <NA>          <NA>
       14            NA    NA      NA <NA>          <NA>
       15            NA    NA      NA <NA>          <NA>
       16            NA    NA      NA <NA>          <NA>
       17            NA    NA      NA <NA>          <NA>
       18            NA    NA      NA <NA>          <NA>
       19            NA    NA      NA <NA>          <NA>
       20            NA    NA      NA <NA>          <NA>
       21            NA    NA      NA <NA>          <NA>
       22            NA    NA      NA <NA>          <NA>
       23            NA    NA      NA <NA>          <NA>
       24            NA    NA      NA <NA>          <NA>
       25            NA    NA      NA <NA>          <NA>
       26            NA    NA      NA <NA>          <NA>
       27            NA    NA      NA <NA>          <NA>
       28            NA    NA      NA <NA>          <NA>
       29            NA    NA      NA <NA>          <NA>
       30            NA    NA      NA <NA>          <NA>
       31            NA    NA      NA <NA>          <NA>
       32             0     0       0 annual_lookup fixed_annual
       33             0     0       0 annual_lookup fixed_annual
       34             0     0       0 annual_lookup fixed_annual
       35             0     0       0 annual_lookup fixed_annual
       36             0     0       0 annual_lookup fixed_annual
       37             0     0       0 annual_lookup fixed_annual
       38             0     0       0 annual_lookup fixed_annual
       39             0     0       0 annual_lookup fixed_annual
       40             0     0       0 annual_lookup fixed_annual
       41             0     0       0 annual_lookup fixed_annual
       42             0     0       0 annual_lookup fixed_annual
       43             0     0       0 annual_lookup fixed_annual
       44             0   365       1 annual_lookup fixed_annual
       45             0   365       1 annual_lookup fixed_annual
       46             0   365       1 annual_lookup fixed_annual
       47             0   365       1 annual_lookup fixed_annual
       48             0   365       1 annual_lookup fixed_annual
       49             0   365       1 annual_lookup fixed_annual
       50             0   365       1 annual_lookup fixed_annual
       51             0   365       1 annual_lookup fixed_annual
       52             0   365       1 annual_lookup fixed_annual
       53             0   365       1 annual_lookup fixed_annual
       54             0   365       1 annual_lookup fixed_annual
       55             0   365       1 annual_lookup fixed_annual
       56             0   730       2 annual_lookup fixed_annual
       57             0   730       2 annual_lookup fixed_annual
       58             0   730       2 annual_lookup fixed_annual
       59             0   730       2 annual_lookup fixed_annual
       60             0   730       2 annual_lookup fixed_annual
       61             0   730       2 annual_lookup fixed_annual
       62             0   730       2 annual_lookup fixed_annual
       63             0   730       2 annual_lookup fixed_annual
       64             0   730       2 annual_lookup fixed_annual
       65             0   730       2 annual_lookup fixed_annual
       66             0   730       2 annual_lookup fixed_annual
       67             0   730       2 annual_lookup fixed_annual
       68             0  1095       3 annual_lookup fixed_annual
       69             0  1095       3 annual_lookup fixed_annual
       70             0  1095       3 annual_lookup fixed_annual
       71             0  1095       3 annual_lookup fixed_annual
       72             0  1095       3 annual_lookup fixed_annual
       73             0  1095       3 annual_lookup fixed_annual
       74             0  1095       3 annual_lookup fixed_annual
       75             0  1095       3 annual_lookup fixed_annual
       76             0  1095       3 annual_lookup fixed_annual
       77             0  1095       3 annual_lookup fixed_annual
       78             0  1095       3 annual_lookup fixed_annual
       79             0  1095       3 annual_lookup fixed_annual
       80             0  1461       4 annual_lookup fixed_annual
       81             0  1461       4 annual_lookup fixed_annual
       82             0  1461       4 annual_lookup fixed_annual
       83             0  1461       4 annual_lookup fixed_annual
       84             0  1461       4 annual_lookup fixed_annual
       85             0  1461       4 annual_lookup fixed_annual
       86             0  1461       4 annual_lookup fixed_annual
       87             0  1461       4 annual_lookup fixed_annual
       88             0  1461       4 annual_lookup fixed_annual
       89             0  1461       4 annual_lookup fixed_annual
       90             0  1461       4 annual_lookup fixed_annual
       91             0  1461       4 annual_lookup fixed_annual
       92             0  1826       5 annual_lookup fixed_annual
       93             0  1826       5 annual_lookup fixed_annual
       94             0  1826       5 annual_lookup fixed_annual
       95             0  1826       5 annual_lookup fixed_annual
       96             0  1826       5 annual_lookup fixed_annual
       97             0  1826       5 annual_lookup fixed_annual
       98             0  1826       5 annual_lookup fixed_annual
       99             0  1826       5 annual_lookup fixed_annual
      100             0  1826       5 annual_lookup fixed_annual
      101             0  1826       5 annual_lookup fixed_annual
      102             0  1826       5 annual_lookup fixed_annual
      103             0  1826       5 annual_lookup fixed_annual
      104             0  2191       6 annual_lookup fixed_annual
      105             0  2191       6 annual_lookup fixed_annual
      106             0  2191       6 annual_lookup fixed_annual
      107             0  2191       6 annual_lookup fixed_annual
      108             0  2191       6 annual_lookup fixed_annual
      109             0  2191       6 annual_lookup fixed_annual
      110             0  2191       6 annual_lookup fixed_annual
      111             0  2191       6 annual_lookup fixed_annual
      112             0  2191       6 annual_lookup fixed_annual
      113             0  2191       6 annual_lookup fixed_annual
      114             0  2191       6 annual_lookup fixed_annual
      115             0  2191       6 annual_lookup fixed_annual
      116             0  2556       7 annual_lookup fixed_annual
      117             0  2556       7 annual_lookup fixed_annual
      118             0  2556       7 annual_lookup fixed_annual
      119             0  2556       7 annual_lookup fixed_annual
      120             0  2556       7 annual_lookup fixed_annual
      121             0  2556       7 annual_lookup fixed_annual
      122             0  2556       7 annual_lookup fixed_annual
      123             0  2556       7 annual_lookup fixed_annual
      124             0  2556       7 annual_lookup fixed_annual
      125             0  2556       7 annual_lookup fixed_annual
      126             0  2556       7 annual_lookup fixed_annual
      127             0  2556       7 annual_lookup fixed_annual
      128             0  2922       8 annual_lookup fixed_annual
      129             0  2922       8 annual_lookup fixed_annual
      130             0  2922       8 annual_lookup fixed_annual
      131             0  2922       8 annual_lookup fixed_annual
      132             0  2922       8 annual_lookup fixed_annual
      133             0  2922       8 annual_lookup fixed_annual
      134             0  2922       8 annual_lookup fixed_annual
      135             0  2922       8 annual_lookup fixed_annual
      136             0  2922       8 annual_lookup fixed_annual
      137             0  2922       8 annual_lookup fixed_annual
      138             0  2922       8 annual_lookup fixed_annual
      139             0  2922       8 annual_lookup fixed_annual
      140             0  3287       9 annual_lookup fixed_annual
      141             0  3287       9 annual_lookup fixed_annual
      142             0  3287       9 annual_lookup fixed_annual
      143             0  3287       9 annual_lookup fixed_annual
      144             0  3287       9 annual_lookup fixed_annual
      145             0  3287       9 annual_lookup fixed_annual
      146             0  3287       9 annual_lookup fixed_annual
      147             0  3287       9 annual_lookup fixed_annual
      148             0  3287       9 annual_lookup fixed_annual
      149             0  3287       9 annual_lookup fixed_annual
      150             0  3287       9 annual_lookup fixed_annual
      151             0  3287       9 annual_lookup fixed_annual
      152             0  3652      10 annual_lookup fixed_annual
      153             0  3652      10 annual_lookup fixed_annual
      154             0  3652      10 annual_lookup fixed_annual
      155             0  3652      10 annual_lookup fixed_annual
      156             0  3652      10 annual_lookup fixed_annual
      157             0  3652      10 annual_lookup fixed_annual
      158             0  3652      10 annual_lookup fixed_annual
      159             0  3652      10 annual_lookup fixed_annual
      160             0  3652      10 annual_lookup fixed_annual
      161             0  3652      10 annual_lookup fixed_annual
      162             0  3652      10 annual_lookup fixed_annual
      163             0  3652      10 annual_lookup fixed_annual
      164             0  4017      11 annual_lookup fixed_annual
      165             0  4017      11 annual_lookup fixed_annual
      166             0  4017      11 annual_lookup fixed_annual
      167             0  4017      11 annual_lookup fixed_annual
      168             0  4017      11 annual_lookup fixed_annual
      169             0  4017      11 annual_lookup fixed_annual
      170             0  4017      11 annual_lookup fixed_annual
      171             0  4017      11 annual_lookup fixed_annual
      172             0  4017      11 annual_lookup fixed_annual
      173             0  4017      11 annual_lookup fixed_annual
      174             0  4017      11 annual_lookup fixed_annual
      175             0  4017      11 annual_lookup fixed_annual
      176             0  4383      12 annual_lookup fixed_annual
      177             0  4383      12 annual_lookup fixed_annual
      178             0  4383      12 annual_lookup fixed_annual
      179             0  4383      12 annual_lookup fixed_annual
      180             0  4383      12 annual_lookup fixed_annual
      181             0  4383      12 annual_lookup fixed_annual
      182             0  4383      12 annual_lookup fixed_annual
      183             0  4383      12 annual_lookup fixed_annual
      184             0  4383      12 annual_lookup fixed_annual
      185             0  4383      12 annual_lookup fixed_annual
      186             0  4383      12 annual_lookup fixed_annual
      187             0  4383      12 annual_lookup fixed_annual
      188             0  4748      13 annual_lookup fixed_annual
      189             0  4748      13 annual_lookup fixed_annual
      190             0  4748      13 annual_lookup fixed_annual
      191             0  4748      13 annual_lookup fixed_annual
      192             0  4748      13 annual_lookup fixed_annual
      193             0  4748      13 annual_lookup fixed_annual
      194             0  4748      13 annual_lookup fixed_annual
      195             0  4748      13 annual_lookup fixed_annual
      196             0  4748      13 annual_lookup fixed_annual
      197             0  4748      13 annual_lookup fixed_annual
      198             0  4748      13 annual_lookup fixed_annual
      199             0  4748      13 annual_lookup fixed_annual
      200             0  5113      14 annual_lookup fixed_annual
      201             0  5113      14 annual_lookup fixed_annual
      202             0  5113      14 annual_lookup fixed_annual
      203             0  5113      14 annual_lookup fixed_annual
      204             0  5113      14 annual_lookup fixed_annual
      205             0  5113      14 annual_lookup fixed_annual
      206             0  5113      14 annual_lookup fixed_annual
      207             0  5113      14 annual_lookup fixed_annual
      208             0  5113      14 annual_lookup fixed_annual
      209             0  5113      14 annual_lookup fixed_annual
      210             0  5113      14 annual_lookup fixed_annual
      211             0  5113      14 annual_lookup fixed_annual
      212             0  5478      15 annual_lookup fixed_annual
      213             0  5478      15 annual_lookup fixed_annual
      214             0  5478      15 annual_lookup fixed_annual
      215             0  5478      15 annual_lookup fixed_annual
      216             0  5478      15 annual_lookup fixed_annual
      217             0  5478      15 annual_lookup fixed_annual
      218             0  5478      15 annual_lookup fixed_annual
      219             0  5478      15 annual_lookup fixed_annual
      220             0  5478      15 annual_lookup fixed_annual
      221             0  5478      15 annual_lookup fixed_annual
      222             0  5478      15 annual_lookup fixed_annual
      223             0  5478      15 annual_lookup fixed_annual
      224             0  5844      16 annual_lookup fixed_annual
      225             0  5844      16 annual_lookup fixed_annual
      226             0  5844      16 annual_lookup fixed_annual
      227             0  5844      16 annual_lookup fixed_annual
      228             0  5844      16 annual_lookup fixed_annual
      229             0  5844      16 annual_lookup fixed_annual
      230             0  5844      16 annual_lookup fixed_annual
      231             0  5844      16 annual_lookup fixed_annual
      232             0  5844      16 annual_lookup fixed_annual
      233             0  5844      16 annual_lookup fixed_annual
      234             0  5844      16 annual_lookup fixed_annual
      235             0  5844      16 annual_lookup fixed_annual
      236             0  6209      17 annual_lookup fixed_annual
      237             0  6209      17 annual_lookup fixed_annual
      238             0  6209      17 annual_lookup fixed_annual
      239             0  6209      17 annual_lookup fixed_annual
      240             0  6209      17 annual_lookup fixed_annual
      241             0  6209      17 annual_lookup fixed_annual
      242             0  6209      17 annual_lookup fixed_annual
      243             0  6209      17 annual_lookup fixed_annual
      244             0  6209      17 annual_lookup fixed_annual
      245             0  6209      17 annual_lookup fixed_annual
      246             0  6209      17 annual_lookup fixed_annual
      247             0  6209      17 annual_lookup fixed_annual
      248             0  6574      18 annual_lookup fixed_annual
      249             0  6574      18 annual_lookup fixed_annual
      250             0  6574      18 annual_lookup fixed_annual
      251             0  6574      18 annual_lookup fixed_annual
      252             0  6574      18 annual_lookup fixed_annual
      253             0  6574      18 annual_lookup fixed_annual
      254             0  6574      18 annual_lookup fixed_annual
      255             0  6574      18 annual_lookup fixed_annual
      256             0  6574      18 annual_lookup fixed_annual
      257             0  6574      18 annual_lookup fixed_annual
      258             0  6574      18 annual_lookup fixed_annual
      259             0  6574      18 annual_lookup fixed_annual
      260             0  6939      19 annual_lookup fixed_annual
      261             0  6939      19 annual_lookup fixed_annual
      262             0  6939      19 annual_lookup fixed_annual
      263             0  6939      19 annual_lookup fixed_annual
      264             0  6939      19 annual_lookup fixed_annual
      265             0  6939      19 annual_lookup fixed_annual
      266             0  6939      19 annual_lookup fixed_annual
      267             0  6939      19 annual_lookup fixed_annual
      268             0  6939      19 annual_lookup fixed_annual
      269             0  6939      19 annual_lookup fixed_annual
      270             0  6939      19 annual_lookup fixed_annual
      271             0  6939      19 annual_lookup fixed_annual
      272             0  7305      20 annual_lookup fixed_annual
      273             0  7305      20 annual_lookup fixed_annual
      274             0  7305      20 annual_lookup fixed_annual
      275             0  7305      20 annual_lookup fixed_annual
      276             0  7305      20 annual_lookup fixed_annual
      277             0  7305      20 annual_lookup fixed_annual
      278             0  7305      20 annual_lookup fixed_annual
      279             0  7305      20 annual_lookup fixed_annual
      280             0  7305      20 annual_lookup fixed_annual
      281             0  7305      20 annual_lookup fixed_annual
      282             0  7305      20 annual_lookup fixed_annual
      283             0  7305      20 annual_lookup fixed_annual
      284             0  7670      21 annual_lookup fixed_annual
      285             0  7670      21 annual_lookup fixed_annual
      286             0  7670      21 annual_lookup fixed_annual
      287             0  7670      21 annual_lookup fixed_annual
      288             0  7670      21 annual_lookup fixed_annual
      289             0  7670      21 annual_lookup fixed_annual
      290             0  7670      21 annual_lookup fixed_annual
      291             0  7670      21 annual_lookup fixed_annual
      292             0  7670      21 annual_lookup fixed_annual
      293             0  7670      21 annual_lookup fixed_annual
      294             0  7670      21 annual_lookup fixed_annual
      295             0  7670      21 annual_lookup fixed_annual
      296             0  8035      22 annual_lookup fixed_annual
      297             0  8035      22 annual_lookup fixed_annual
      298             0  8035      22 annual_lookup fixed_annual
      299             0  8035      22 annual_lookup fixed_annual
      300             0  8035      22 annual_lookup fixed_annual
      301             0  8035      22 annual_lookup fixed_annual
      302             0  8035      22 annual_lookup fixed_annual
      303             0  8035      22 annual_lookup fixed_annual
      304             0  8035      22 annual_lookup fixed_annual
      305             0  8035      22 annual_lookup fixed_annual
      306             0  8035      22 annual_lookup fixed_annual
      307             0  8035      22 annual_lookup fixed_annual
      308             0  8400      23 annual_lookup fixed_annual
      309             0  8400      23 annual_lookup fixed_annual
      310             0  8400      23 annual_lookup fixed_annual
      311             0  8400      23 annual_lookup fixed_annual
      312             0  8400      23 annual_lookup fixed_annual
      313             0  8400      23 annual_lookup fixed_annual
      314             0  8400      23 annual_lookup fixed_annual
      315             0  8400      23 annual_lookup fixed_annual
      316             0  8400      23 annual_lookup fixed_annual
      317             0  8400      23 annual_lookup fixed_annual
      318             0  8400      23 annual_lookup fixed_annual
      319             0  8400      23 annual_lookup fixed_annual
      320             0  8766      24 annual_lookup fixed_annual
          maturity_timing prediction_timing
          <chr>           <chr>
        1 <NA>            <NA>
        2 <NA>            <NA>
        3 <NA>            <NA>
        4 <NA>            <NA>
        5 <NA>            <NA>
        6 <NA>            <NA>
        7 <NA>            <NA>
        8 <NA>            <NA>
        9 <NA>            <NA>
       10 <NA>            <NA>
       11 <NA>            <NA>
       12 <NA>            <NA>
       13 <NA>            <NA>
       14 <NA>            <NA>
       15 <NA>            <NA>
       16 <NA>            <NA>
       17 <NA>            <NA>
       18 <NA>            <NA>
       19 <NA>            <NA>
       20 <NA>            <NA>
       21 <NA>            <NA>
       22 <NA>            <NA>
       23 <NA>            <NA>
       24 <NA>            <NA>
       25 <NA>            <NA>
       26 <NA>            <NA>
       27 <NA>            <NA>
       28 <NA>            <NA>
       29 <NA>            <NA>
       30 <NA>            <NA>
       31 <NA>            <NA>
       32 fractional_age  annual_interval
       33 fractional_age  annual_interval
       34 fractional_age  annual_interval
       35 fractional_age  annual_interval
       36 fractional_age  annual_interval
       37 fractional_age  annual_interval
       38 fractional_age  annual_interval
       39 fractional_age  annual_interval
       40 fractional_age  annual_interval
       41 fractional_age  annual_interval
       42 fractional_age  annual_interval
       43 fractional_age  annual_interval
       44 fractional_age  annual_interval
       45 fractional_age  annual_interval
       46 fractional_age  annual_interval
       47 fractional_age  annual_interval
       48 fractional_age  annual_interval
       49 fractional_age  annual_interval
       50 fractional_age  annual_interval
       51 fractional_age  annual_interval
       52 fractional_age  annual_interval
       53 fractional_age  annual_interval
       54 fractional_age  annual_interval
       55 fractional_age  annual_interval
       56 fractional_age  annual_interval
       57 fractional_age  annual_interval
       58 fractional_age  annual_interval
       59 fractional_age  annual_interval
       60 fractional_age  annual_interval
       61 fractional_age  annual_interval
       62 fractional_age  annual_interval
       63 fractional_age  annual_interval
       64 fractional_age  annual_interval
       65 fractional_age  annual_interval
       66 fractional_age  annual_interval
       67 fractional_age  annual_interval
       68 fractional_age  annual_interval
       69 fractional_age  annual_interval
       70 fractional_age  annual_interval
       71 fractional_age  annual_interval
       72 fractional_age  annual_interval
       73 fractional_age  annual_interval
       74 fractional_age  annual_interval
       75 fractional_age  annual_interval
       76 fractional_age  annual_interval
       77 fractional_age  annual_interval
       78 fractional_age  annual_interval
       79 fractional_age  annual_interval
       80 fractional_age  annual_interval
       81 fractional_age  annual_interval
       82 fractional_age  annual_interval
       83 fractional_age  annual_interval
       84 fractional_age  annual_interval
       85 fractional_age  annual_interval
       86 fractional_age  annual_interval
       87 fractional_age  annual_interval
       88 fractional_age  annual_interval
       89 fractional_age  annual_interval
       90 fractional_age  annual_interval
       91 fractional_age  annual_interval
       92 fractional_age  annual_interval
       93 fractional_age  annual_interval
       94 fractional_age  annual_interval
       95 fractional_age  annual_interval
       96 fractional_age  annual_interval
       97 fractional_age  annual_interval
       98 fractional_age  annual_interval
       99 fractional_age  annual_interval
      100 fractional_age  annual_interval
      101 fractional_age  annual_interval
      102 fractional_age  annual_interval
      103 fractional_age  annual_interval
      104 fractional_age  annual_interval
      105 fractional_age  annual_interval
      106 fractional_age  annual_interval
      107 fractional_age  annual_interval
      108 fractional_age  annual_interval
      109 fractional_age  annual_interval
      110 fractional_age  annual_interval
      111 fractional_age  annual_interval
      112 fractional_age  annual_interval
      113 fractional_age  annual_interval
      114 fractional_age  annual_interval
      115 fractional_age  annual_interval
      116 fractional_age  annual_interval
      117 fractional_age  annual_interval
      118 fractional_age  annual_interval
      119 fractional_age  annual_interval
      120 fractional_age  annual_interval
      121 fractional_age  annual_interval
      122 fractional_age  annual_interval
      123 fractional_age  annual_interval
      124 fractional_age  annual_interval
      125 fractional_age  annual_interval
      126 fractional_age  annual_interval
      127 fractional_age  annual_interval
      128 fractional_age  annual_interval
      129 fractional_age  annual_interval
      130 fractional_age  annual_interval
      131 fractional_age  annual_interval
      132 fractional_age  annual_interval
      133 fractional_age  annual_interval
      134 fractional_age  annual_interval
      135 fractional_age  annual_interval
      136 fractional_age  annual_interval
      137 fractional_age  annual_interval
      138 fractional_age  annual_interval
      139 fractional_age  annual_interval
      140 fractional_age  annual_interval
      141 fractional_age  annual_interval
      142 fractional_age  annual_interval
      143 fractional_age  annual_interval
      144 fractional_age  annual_interval
      145 fractional_age  annual_interval
      146 fractional_age  annual_interval
      147 fractional_age  annual_interval
      148 fractional_age  annual_interval
      149 fractional_age  annual_interval
      150 fractional_age  annual_interval
      151 fractional_age  annual_interval
      152 fractional_age  annual_interval
      153 fractional_age  annual_interval
      154 fractional_age  annual_interval
      155 fractional_age  annual_interval
      156 fractional_age  annual_interval
      157 fractional_age  annual_interval
      158 fractional_age  annual_interval
      159 fractional_age  annual_interval
      160 fractional_age  annual_interval
      161 fractional_age  annual_interval
      162 fractional_age  annual_interval
      163 fractional_age  annual_interval
      164 fractional_age  annual_interval
      165 fractional_age  annual_interval
      166 fractional_age  annual_interval
      167 fractional_age  annual_interval
      168 fractional_age  annual_interval
      169 fractional_age  annual_interval
      170 fractional_age  annual_interval
      171 fractional_age  annual_interval
      172 fractional_age  annual_interval
      173 fractional_age  annual_interval
      174 fractional_age  annual_interval
      175 fractional_age  annual_interval
      176 fractional_age  annual_interval
      177 fractional_age  annual_interval
      178 fractional_age  annual_interval
      179 fractional_age  annual_interval
      180 fractional_age  annual_interval
      181 fractional_age  annual_interval
      182 fractional_age  annual_interval
      183 fractional_age  annual_interval
      184 fractional_age  annual_interval
      185 fractional_age  annual_interval
      186 fractional_age  annual_interval
      187 fractional_age  annual_interval
      188 fractional_age  annual_interval
      189 fractional_age  annual_interval
      190 fractional_age  annual_interval
      191 fractional_age  annual_interval
      192 fractional_age  annual_interval
      193 fractional_age  annual_interval
      194 fractional_age  annual_interval
      195 fractional_age  annual_interval
      196 fractional_age  annual_interval
      197 fractional_age  annual_interval
      198 fractional_age  annual_interval
      199 fractional_age  annual_interval
      200 fractional_age  annual_interval
      201 fractional_age  annual_interval
      202 fractional_age  annual_interval
      203 fractional_age  annual_interval
      204 fractional_age  annual_interval
      205 fractional_age  annual_interval
      206 fractional_age  annual_interval
      207 fractional_age  annual_interval
      208 fractional_age  annual_interval
      209 fractional_age  annual_interval
      210 fractional_age  annual_interval
      211 fractional_age  annual_interval
      212 fractional_age  annual_interval
      213 fractional_age  annual_interval
      214 fractional_age  annual_interval
      215 fractional_age  annual_interval
      216 fractional_age  annual_interval
      217 fractional_age  annual_interval
      218 fractional_age  annual_interval
      219 fractional_age  annual_interval
      220 fractional_age  annual_interval
      221 fractional_age  annual_interval
      222 fractional_age  annual_interval
      223 fractional_age  annual_interval
      224 fractional_age  annual_interval
      225 fractional_age  annual_interval
      226 fractional_age  annual_interval
      227 fractional_age  annual_interval
      228 fractional_age  annual_interval
      229 fractional_age  annual_interval
      230 fractional_age  annual_interval
      231 fractional_age  annual_interval
      232 fractional_age  annual_interval
      233 fractional_age  annual_interval
      234 fractional_age  annual_interval
      235 fractional_age  annual_interval
      236 fractional_age  annual_interval
      237 fractional_age  annual_interval
      238 fractional_age  annual_interval
      239 fractional_age  annual_interval
      240 fractional_age  annual_interval
      241 fractional_age  annual_interval
      242 fractional_age  annual_interval
      243 fractional_age  annual_interval
      244 fractional_age  annual_interval
      245 fractional_age  annual_interval
      246 fractional_age  annual_interval
      247 fractional_age  annual_interval
      248 fractional_age  annual_interval
      249 fractional_age  annual_interval
      250 fractional_age  annual_interval
      251 fractional_age  annual_interval
      252 fractional_age  annual_interval
      253 fractional_age  annual_interval
      254 fractional_age  annual_interval
      255 fractional_age  annual_interval
      256 fractional_age  annual_interval
      257 fractional_age  annual_interval
      258 fractional_age  annual_interval
      259 fractional_age  annual_interval
      260 fractional_age  annual_interval
      261 fractional_age  annual_interval
      262 fractional_age  annual_interval
      263 fractional_age  annual_interval
      264 fractional_age  annual_interval
      265 fractional_age  annual_interval
      266 fractional_age  annual_interval
      267 fractional_age  annual_interval
      268 fractional_age  annual_interval
      269 fractional_age  annual_interval
      270 fractional_age  annual_interval
      271 fractional_age  annual_interval
      272 fractional_age  annual_interval
      273 fractional_age  annual_interval
      274 fractional_age  annual_interval
      275 fractional_age  annual_interval
      276 fractional_age  annual_interval
      277 fractional_age  annual_interval
      278 fractional_age  annual_interval
      279 fractional_age  annual_interval
      280 fractional_age  annual_interval
      281 fractional_age  annual_interval
      282 fractional_age  annual_interval
      283 fractional_age  annual_interval
      284 fractional_age  annual_interval
      285 fractional_age  annual_interval
      286 fractional_age  annual_interval
      287 fractional_age  annual_interval
      288 fractional_age  annual_interval
      289 fractional_age  annual_interval
      290 fractional_age  annual_interval
      291 fractional_age  annual_interval
      292 fractional_age  annual_interval
      293 fractional_age  annual_interval
      294 fractional_age  annual_interval
      295 fractional_age  annual_interval
      296 fractional_age  annual_interval
      297 fractional_age  annual_interval
      298 fractional_age  annual_interval
      299 fractional_age  annual_interval
      300 fractional_age  annual_interval
      301 fractional_age  annual_interval
      302 fractional_age  annual_interval
      303 fractional_age  annual_interval
      304 fractional_age  annual_interval
      305 fractional_age  annual_interval
      306 fractional_age  annual_interval
      307 fractional_age  annual_interval
      308 fractional_age  annual_interval
      309 fractional_age  annual_interval
      310 fractional_age  annual_interval
      311 fractional_age  annual_interval
      312 fractional_age  annual_interval
      313 fractional_age  annual_interval
      314 fractional_age  annual_interval
      315 fractional_age  annual_interval
      316 fractional_age  annual_interval
      317 fractional_age  annual_interval
      318 fractional_age  annual_interval
      319 fractional_age  annual_interval
      320 fractional_age  annual_interval
      # i 13,676 more rows

# `get_estimates()` works with estimation run

    Code
      estimates_snapshot <- dplyr::mutate(dplyr::select(get_estimates(readRDS(
        fit_files[[1]])), -estimated, -expected, -uncertainty, -gradient, -likelihood,
      -log_like_cv), dplyr::across(c(date, interval_end), observation_date_iso))
      estimates_snapshot_lines <- capture.output(suppressMessages(print(
        estimates_snapshot, n = 320, width = Inf)))
      cat(trimws(estimates_snapshot_lines, which = "right"), sep = "\n")
    Output
      # A tibble: 13,996 x 35
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
          parameter_id fleet  year_i sample_i age_i length_i input observed
                 <int> <chr>   <int>    <int> <int>    <int> <dbl>    <dbl>
        1            4 fleet1      1       NA    NA       NA -6.99       NA
        2           24 fleet1      2       NA    NA       NA -5.40       NA
        3           25 fleet1      3       NA    NA       NA -4.65       NA
        4           26 fleet1      4       NA    NA       NA -4.19       NA
        5           27 fleet1      5       NA    NA       NA -4.54       NA
        6           28 fleet1      6       NA    NA       NA -3.66       NA
        7           29 fleet1      7       NA    NA       NA -3.64       NA
        8           30 fleet1      8       NA    NA       NA -2.52       NA
        9           31 fleet1      9       NA    NA       NA -3.32       NA
       10           32 fleet1     10       NA    NA       NA -3.03       NA
       11           33 fleet1     11       NA    NA       NA -2.84       NA
       12           34 fleet1     12       NA    NA       NA -2.74       NA
       13           35 fleet1     13       NA    NA       NA -3.22       NA
       14           36 fleet1     14       NA    NA       NA -2.66       NA
       15           37 fleet1     15       NA    NA       NA -2.57       NA
       16           38 fleet1     16       NA    NA       NA -2.74       NA
       17           39 fleet1     17       NA    NA       NA -1.73       NA
       18           40 fleet1     18       NA    NA       NA -2.04       NA
       19           41 fleet1     19       NA    NA       NA -2.05       NA
       20           42 fleet1     20       NA    NA       NA -2.07       NA
       21           43 fleet1     21       NA    NA       NA -1.58       NA
       22           44 fleet1     22       NA    NA       NA -2.06       NA
       23           45 fleet1     23       NA    NA       NA -1.31       NA
       24           46 fleet1     24       NA    NA       NA -1.59       NA
       25           47 fleet1     25       NA    NA       NA -1.60       NA
       26           48 fleet1     26       NA    NA       NA -1.74       NA
       27           49 fleet1     27       NA    NA       NA -1.77       NA
       28           50 fleet1     28       NA    NA       NA -1.26       NA
       29           51 fleet1     29       NA    NA       NA -1.67       NA
       30           52 fleet1     30       NA    NA       NA -1.04       NA
       31            3 fleet1     NA       NA    NA       NA  0          NA
       32           NA fleet1      1        1     1       NA NA          14
       33           NA fleet1      1        1     2       NA NA          20
       34           NA fleet1      1        1     3       NA NA          23
       35           NA fleet1      1        1     4       NA NA          30
       36           NA fleet1      1        1     5       NA NA          20
       37           NA fleet1      1        1     6       NA NA          10
       38           NA fleet1      1        1     7       NA NA          22
       39           NA fleet1      1        1     8       NA NA          11
       40           NA fleet1      1        1     9       NA NA          12
       41           NA fleet1      1        1    10       NA NA           6
       42           NA fleet1      1        1    11       NA NA           7
       43           NA fleet1      1        1    12       NA NA          25
       44           NA fleet1      2        2     1       NA NA          21
       45           NA fleet1      2        2     2       NA NA          19
       46           NA fleet1      2        2     3       NA NA          22
       47           NA fleet1      2        2     4       NA NA          18
       48           NA fleet1      2        2     5       NA NA          31
       49           NA fleet1      2        2     6       NA NA          11
       50           NA fleet1      2        2     7       NA NA          13
       51           NA fleet1      2        2     8       NA NA          17
       52           NA fleet1      2        2     9       NA NA           9
       53           NA fleet1      2        2    10       NA NA           7
       54           NA fleet1      2        2    11       NA NA           5
       55           NA fleet1      2        2    12       NA NA          27
       56           NA fleet1      3        3     1       NA NA          11
       57           NA fleet1      3        3     2       NA NA          30
       58           NA fleet1      3        3     3       NA NA          21
       59           NA fleet1      3        3     4       NA NA          23
       60           NA fleet1      3        3     5       NA NA          19
       61           NA fleet1      3        3     6       NA NA          18
       62           NA fleet1      3        3     7       NA NA          14
       63           NA fleet1      3        3     8       NA NA          17
       64           NA fleet1      3        3     9       NA NA           6
       65           NA fleet1      3        3    10       NA NA           9
       66           NA fleet1      3        3    11       NA NA           6
       67           NA fleet1      3        3    12       NA NA          26
       68           NA fleet1      4        4     1       NA NA          10
       69           NA fleet1      4        4     2       NA NA          11
       70           NA fleet1      4        4     3       NA NA          44
       71           NA fleet1      4        4     4       NA NA          20
       72           NA fleet1      4        4     5       NA NA          20
       73           NA fleet1      4        4     6       NA NA          21
       74           NA fleet1      4        4     7       NA NA          13
       75           NA fleet1      4        4     8       NA NA          11
       76           NA fleet1      4        4     9       NA NA          13
       77           NA fleet1      4        4    10       NA NA           6
       78           NA fleet1      4        4    11       NA NA           3
       79           NA fleet1      4        4    12       NA NA          28
       80           NA fleet1      5        5     1       NA NA          31
       81           NA fleet1      5        5     2       NA NA          10
       82           NA fleet1      5        5     3       NA NA          32
       83           NA fleet1      5        5     4       NA NA          34
       84           NA fleet1      5        5     5       NA NA          18
       85           NA fleet1      5        5     6       NA NA          19
       86           NA fleet1      5        5     7       NA NA           9
       87           NA fleet1      5        5     8       NA NA           9
       88           NA fleet1      5        5     9       NA NA           6
       89           NA fleet1      5        5    10       NA NA           4
       90           NA fleet1      5        5    11       NA NA           6
       91           NA fleet1      5        5    12       NA NA          22
       92           NA fleet1      6        6     1       NA NA          16
       93           NA fleet1      6        6     2       NA NA          37
       94           NA fleet1      6        6     3       NA NA          14
       95           NA fleet1      6        6     4       NA NA          18
       96           NA fleet1      6        6     5       NA NA          29
       97           NA fleet1      6        6     6       NA NA          15
       98           NA fleet1      6        6     7       NA NA          12
       99           NA fleet1      6        6     8       NA NA          13
      100           NA fleet1      6        6     9       NA NA           8
      101           NA fleet1      6        6    10       NA NA           9
      102           NA fleet1      6        6    11       NA NA           5
      103           NA fleet1      6        6    12       NA NA          24
      104           NA fleet1      7        7     1       NA NA          11
      105           NA fleet1      7        7     2       NA NA          26
      106           NA fleet1      7        7     3       NA NA          44
      107           NA fleet1      7        7     4       NA NA          17
      108           NA fleet1      7        7     5       NA NA          19
      109           NA fleet1      7        7     6       NA NA          28
      110           NA fleet1      7        7     7       NA NA           7
      111           NA fleet1      7        7     8       NA NA           8
      112           NA fleet1      7        7     9       NA NA           7
      113           NA fleet1      7        7    10       NA NA           4
      114           NA fleet1      7        7    11       NA NA           5
      115           NA fleet1      7        7    12       NA NA          24
      116           NA fleet1      8        8     1       NA NA          18
      117           NA fleet1      8        8     2       NA NA          15
      118           NA fleet1      8        8     3       NA NA          44
      119           NA fleet1      8        8     4       NA NA          40
      120           NA fleet1      8        8     5       NA NA          12
      121           NA fleet1      8        8     6       NA NA           8
      122           NA fleet1      8        8     7       NA NA          14
      123           NA fleet1      8        8     8       NA NA          13
      124           NA fleet1      8        8     9       NA NA           5
      125           NA fleet1      8        8    10       NA NA           5
      126           NA fleet1      8        8    11       NA NA           5
      127           NA fleet1      8        8    12       NA NA          21
      128           NA fleet1      9        9     1       NA NA          14
      129           NA fleet1      9        9     2       NA NA          32
      130           NA fleet1      9        9     3       NA NA          27
      131           NA fleet1      9        9     4       NA NA          35
      132           NA fleet1      9        9     5       NA NA          31
      133           NA fleet1      9        9     6       NA NA           9
      134           NA fleet1      9        9     7       NA NA           9
      135           NA fleet1      9        9     8       NA NA          12
      136           NA fleet1      9        9     9       NA NA           5
      137           NA fleet1      9        9    10       NA NA           5
      138           NA fleet1      9        9    11       NA NA           3
      139           NA fleet1      9        9    12       NA NA          18
      140           NA fleet1     10       10     1       NA NA          15
      141           NA fleet1     10       10     2       NA NA          21
      142           NA fleet1     10       10     3       NA NA          43
      143           NA fleet1     10       10     4       NA NA          20
      144           NA fleet1     10       10     5       NA NA          22
      145           NA fleet1     10       10     6       NA NA          21
      146           NA fleet1     10       10     7       NA NA           7
      147           NA fleet1     10       10     8       NA NA           8
      148           NA fleet1     10       10     9       NA NA          13
      149           NA fleet1     10       10    10       NA NA           4
      150           NA fleet1     10       10    11       NA NA           4
      151           NA fleet1     10       10    12       NA NA          22
      152           NA fleet1     11       11     1       NA NA          15
      153           NA fleet1     11       11     2       NA NA          34
      154           NA fleet1     11       11     3       NA NA          23
      155           NA fleet1     11       11     4       NA NA          43
      156           NA fleet1     11       11     5       NA NA          15
      157           NA fleet1     11       11     6       NA NA          23
      158           NA fleet1     11       11     7       NA NA          21
      159           NA fleet1     11       11     8       NA NA           5
      160           NA fleet1     11       11     9       NA NA           3
      161           NA fleet1     11       11    10       NA NA           5
      162           NA fleet1     11       11    11       NA NA           2
      163           NA fleet1     11       11    12       NA NA          11
      164           NA fleet1     12       12     1       NA NA          11
      165           NA fleet1     12       12     2       NA NA          32
      166           NA fleet1     12       12     3       NA NA          30
      167           NA fleet1     12       12     4       NA NA          26
      168           NA fleet1     12       12     5       NA NA          33
      169           NA fleet1     12       12     6       NA NA          14
      170           NA fleet1     12       12     7       NA NA          13
      171           NA fleet1     12       12     8       NA NA          21
      172           NA fleet1     12       12     9       NA NA           4
      173           NA fleet1     12       12    10       NA NA           3
      174           NA fleet1     12       12    11       NA NA           4
      175           NA fleet1     12       12    12       NA NA           9
      176           NA fleet1     13       13     1       NA NA          22
      177           NA fleet1     13       13     2       NA NA          21
      178           NA fleet1     13       13     3       NA NA          23
      179           NA fleet1     13       13     4       NA NA          28
      180           NA fleet1     13       13     5       NA NA          23
      181           NA fleet1     13       13     6       NA NA          18
      182           NA fleet1     13       13     7       NA NA          15
      183           NA fleet1     13       13     8       NA NA          18
      184           NA fleet1     13       13     9       NA NA          16
      185           NA fleet1     13       13    10       NA NA           3
      186           NA fleet1     13       13    11       NA NA           3
      187           NA fleet1     13       13    12       NA NA          10
      188           NA fleet1     14       14     1       NA NA          23
      189           NA fleet1     14       14     2       NA NA          28
      190           NA fleet1     14       14     3       NA NA          17
      191           NA fleet1     14       14     4       NA NA          41
      192           NA fleet1     14       14     5       NA NA          24
      193           NA fleet1     14       14     6       NA NA          10
      194           NA fleet1     14       14     7       NA NA          10
      195           NA fleet1     14       14     8       NA NA           8
      196           NA fleet1     14       14     9       NA NA           9
      197           NA fleet1     14       14    10       NA NA          12
      198           NA fleet1     14       14    11       NA NA           5
      199           NA fleet1     14       14    12       NA NA          13
      200           NA fleet1     15       15     1       NA NA          14
      201           NA fleet1     15       15     2       NA NA          38
      202           NA fleet1     15       15     3       NA NA          39
      203           NA fleet1     15       15     4       NA NA          16
      204           NA fleet1     15       15     5       NA NA          28
      205           NA fleet1     15       15     6       NA NA          21
      206           NA fleet1     15       15     7       NA NA          10
      207           NA fleet1     15       15     8       NA NA          11
      208           NA fleet1     15       15     9       NA NA           5
      209           NA fleet1     15       15    10       NA NA           4
      210           NA fleet1     15       15    11       NA NA           0
      211           NA fleet1     15       15    12       NA NA          14
      212           NA fleet1     16       16     1       NA NA          29
      213           NA fleet1     16       16     2       NA NA          25
      214           NA fleet1     16       16     3       NA NA          39
      215           NA fleet1     16       16     4       NA NA          25
      216           NA fleet1     16       16     5       NA NA          17
      217           NA fleet1     16       16     6       NA NA          13
      218           NA fleet1     16       16     7       NA NA          12
      219           NA fleet1     16       16     8       NA NA           8
      220           NA fleet1     16       16     9       NA NA           7
      221           NA fleet1     16       16    10       NA NA           3
      222           NA fleet1     16       16    11       NA NA           7
      223           NA fleet1     16       16    12       NA NA          15
      224           NA fleet1     17       17     1       NA NA          33
      225           NA fleet1     17       17     2       NA NA          38
      226           NA fleet1     17       17     3       NA NA          28
      227           NA fleet1     17       17     4       NA NA          29
      228           NA fleet1     17       17     5       NA NA          24
      229           NA fleet1     17       17     6       NA NA           6
      230           NA fleet1     17       17     7       NA NA          10
      231           NA fleet1     17       17     8       NA NA           1
      232           NA fleet1     17       17     9       NA NA           6
      233           NA fleet1     17       17    10       NA NA           5
      234           NA fleet1     17       17    11       NA NA           2
      235           NA fleet1     17       17    12       NA NA          18
      236           NA fleet1     18       18     1       NA NA          17
      237           NA fleet1     18       18     2       NA NA          40
      238           NA fleet1     18       18     3       NA NA          50
      239           NA fleet1     18       18     4       NA NA          25
      240           NA fleet1     18       18     5       NA NA          17
      241           NA fleet1     18       18     6       NA NA          13
      242           NA fleet1     18       18     7       NA NA          11
      243           NA fleet1     18       18     8       NA NA           8
      244           NA fleet1     18       18     9       NA NA           7
      245           NA fleet1     18       18    10       NA NA           1
      246           NA fleet1     18       18    11       NA NA           4
      247           NA fleet1     18       18    12       NA NA           7
      248           NA fleet1     19       19     1       NA NA          26
      249           NA fleet1     19       19     2       NA NA          24
      250           NA fleet1     19       19     3       NA NA          37
      251           NA fleet1     19       19     4       NA NA          34
      252           NA fleet1     19       19     5       NA NA          22
      253           NA fleet1     19       19     6       NA NA          23
      254           NA fleet1     19       19     7       NA NA           8
      255           NA fleet1     19       19     8       NA NA           6
      256           NA fleet1     19       19     9       NA NA           3
      257           NA fleet1     19       19    10       NA NA           5
      258           NA fleet1     19       19    11       NA NA           4
      259           NA fleet1     19       19    12       NA NA           8
      260           NA fleet1     20       20     1       NA NA           9
      261           NA fleet1     20       20     2       NA NA          31
      262           NA fleet1     20       20     3       NA NA          33
      263           NA fleet1     20       20     4       NA NA          51
      264           NA fleet1     20       20     5       NA NA          29
      265           NA fleet1     20       20     6       NA NA          12
      266           NA fleet1     20       20     7       NA NA          11
      267           NA fleet1     20       20     8       NA NA          10
      268           NA fleet1     20       20     9       NA NA           5
      269           NA fleet1     20       20    10       NA NA           2
      270           NA fleet1     20       20    11       NA NA           5
      271           NA fleet1     20       20    12       NA NA           2
      272           NA fleet1     21       21     1       NA NA          49
      273           NA fleet1     21       21     2       NA NA          15
      274           NA fleet1     21       21     3       NA NA          38
      275           NA fleet1     21       21     4       NA NA          23
      276           NA fleet1     21       21     5       NA NA          25
      277           NA fleet1     21       21     6       NA NA          17
      278           NA fleet1     21       21     7       NA NA          10
      279           NA fleet1     21       21     8       NA NA           8
      280           NA fleet1     21       21     9       NA NA           2
      281           NA fleet1     21       21    10       NA NA           1
      282           NA fleet1     21       21    11       NA NA           5
      283           NA fleet1     21       21    12       NA NA           7
      284           NA fleet1     22       22     1       NA NA          40
      285           NA fleet1     22       22     2       NA NA          67
      286           NA fleet1     22       22     3       NA NA          14
      287           NA fleet1     22       22     4       NA NA          23
      288           NA fleet1     22       22     5       NA NA          15
      289           NA fleet1     22       22     6       NA NA           6
      290           NA fleet1     22       22     7       NA NA          14
      291           NA fleet1     22       22     8       NA NA           4
      292           NA fleet1     22       22     9       NA NA           5
      293           NA fleet1     22       22    10       NA NA           4
      294           NA fleet1     22       22    11       NA NA           1
      295           NA fleet1     22       22    12       NA NA           7
      296           NA fleet1     23       23     1       NA NA          22
      297           NA fleet1     23       23     2       NA NA          51
      298           NA fleet1     23       23     3       NA NA          56
      299           NA fleet1     23       23     4       NA NA          16
      300           NA fleet1     23       23     5       NA NA          11
      301           NA fleet1     23       23     6       NA NA           7
      302           NA fleet1     23       23     7       NA NA          18
      303           NA fleet1     23       23     8       NA NA           5
      304           NA fleet1     23       23     9       NA NA           1
      305           NA fleet1     23       23    10       NA NA           3
      306           NA fleet1     23       23    11       NA NA           5
      307           NA fleet1     23       23    12       NA NA           5
      308           NA fleet1     24       24     1       NA NA          24
      309           NA fleet1     24       24     2       NA NA          38
      310           NA fleet1     24       24     3       NA NA          48
      311           NA fleet1     24       24     4       NA NA          36
      312           NA fleet1     24       24     5       NA NA          14
      313           NA fleet1     24       24     6       NA NA          15
      314           NA fleet1     24       24     7       NA NA           4
      315           NA fleet1     24       24     8       NA NA           6
      316           NA fleet1     24       24     9       NA NA           8
      317           NA fleet1     24       24    10       NA NA           1
      318           NA fleet1     24       24    11       NA NA           4
      319           NA fleet1     24       24    12       NA NA           2
      320           NA fleet1     25       25     1       NA NA          36
          estimation_type  distribution input_type  lpdf log_sd timing date
          <chr>            <chr>        <chr>      <dbl>  <dbl>  <int> <chr>
        1 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        2 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        3 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        4 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        5 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        6 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        7 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        8 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
        9 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       10 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       11 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       12 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       13 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       14 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       15 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       16 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       17 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       18 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       19 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       20 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       21 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       22 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       23 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       24 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       25 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       26 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       27 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       28 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       29 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       30 fixed_effects    <NA>         <NA>         NA      NA     NA <NA>
       31 constant         <NA>         <NA>         NA      NA     NA <NA>
       32 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       33 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       34 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       35 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       36 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       37 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       38 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       39 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       40 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       41 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       42 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       43 derived_quantity multinomial  data       -826.     NA      1 0001-01-01
       44 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       45 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       46 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       47 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       48 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       49 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       50 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       51 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       52 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       53 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       54 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       55 derived_quantity multinomial  data       -826.     NA      2 0002-01-01
       56 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       57 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       58 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       59 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       60 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       61 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       62 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       63 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       64 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       65 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       66 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       67 derived_quantity multinomial  data       -826.     NA      3 0003-01-01
       68 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       69 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       70 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       71 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       72 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       73 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       74 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       75 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       76 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       77 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       78 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       79 derived_quantity multinomial  data       -826.     NA      4 0004-01-01
       80 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       81 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       82 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       83 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       84 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       85 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       86 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       87 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       88 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       89 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       90 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       91 derived_quantity multinomial  data       -826.     NA      5 0005-01-01
       92 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       93 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       94 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       95 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       96 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       97 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       98 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
       99 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
      100 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
      101 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
      102 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
      103 derived_quantity multinomial  data       -826.     NA      6 0006-01-01
      104 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      105 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      106 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      107 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      108 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      109 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      110 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      111 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      112 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      113 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      114 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      115 derived_quantity multinomial  data       -826.     NA      7 0007-01-01
      116 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      117 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      118 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      119 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      120 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      121 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      122 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      123 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      124 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      125 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      126 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      127 derived_quantity multinomial  data       -826.     NA      8 0008-01-01
      128 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      129 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      130 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      131 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      132 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      133 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      134 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      135 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      136 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      137 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      138 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      139 derived_quantity multinomial  data       -826.     NA      9 0009-01-01
      140 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      141 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      142 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      143 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      144 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      145 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      146 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      147 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      148 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      149 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      150 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      151 derived_quantity multinomial  data       -826.     NA     10 0010-01-01
      152 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      153 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      154 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      155 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      156 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      157 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      158 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      159 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      160 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      161 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      162 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      163 derived_quantity multinomial  data       -826.     NA     11 0011-01-01
      164 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      165 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      166 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      167 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      168 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      169 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      170 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      171 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      172 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      173 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      174 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      175 derived_quantity multinomial  data       -826.     NA     12 0012-01-01
      176 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      177 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      178 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      179 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      180 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      181 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      182 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      183 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      184 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      185 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      186 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      187 derived_quantity multinomial  data       -826.     NA     13 0013-01-01
      188 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      189 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      190 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      191 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      192 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      193 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      194 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      195 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      196 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      197 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      198 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      199 derived_quantity multinomial  data       -826.     NA     14 0014-01-01
      200 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      201 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      202 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      203 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      204 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      205 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      206 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      207 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      208 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      209 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      210 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      211 derived_quantity multinomial  data       -826.     NA     15 0015-01-01
      212 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      213 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      214 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      215 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      216 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      217 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      218 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      219 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      220 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      221 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      222 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      223 derived_quantity multinomial  data       -826.     NA     16 0016-01-01
      224 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      225 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      226 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      227 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      228 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      229 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      230 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      231 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      232 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      233 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      234 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      235 derived_quantity multinomial  data       -826.     NA     17 0017-01-01
      236 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      237 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      238 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      239 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      240 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      241 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      242 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      243 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      244 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      245 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      246 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      247 derived_quantity multinomial  data       -826.     NA     18 0018-01-01
      248 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      249 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      250 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      251 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      252 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      253 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      254 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      255 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      256 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      257 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      258 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      259 derived_quantity multinomial  data       -826.     NA     19 0019-01-01
      260 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      261 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      262 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      263 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      264 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      265 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      266 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      267 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      268 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      269 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      270 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      271 derived_quantity multinomial  data       -826.     NA     20 0020-01-01
      272 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      273 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      274 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      275 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      276 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      277 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      278 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      279 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      280 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      281 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      282 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      283 derived_quantity multinomial  data       -826.     NA     21 0021-01-01
      284 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      285 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      286 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      287 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      288 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      289 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      290 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      291 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      292 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      293 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      294 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      295 derived_quantity multinomial  data       -826.     NA     22 0022-01-01
      296 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      297 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      298 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      299 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      300 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      301 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      302 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      303 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      304 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      305 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      306 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      307 derived_quantity multinomial  data       -826.     NA     23 0023-01-01
      308 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      309 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      310 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      311 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      312 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      313 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      314 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      315 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      316 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      317 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      318 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      319 derived_quantity multinomial  data       -826.     NA     24 0024-01-01
      320 derived_quantity multinomial  data       -826.     NA     25 0025-01-01
          input_precision prediction_basis support  interval_end
          <chr>           <chr>            <chr>    <chr>
        1 <NA>            <NA>             <NA>     <NA>
        2 <NA>            <NA>             <NA>     <NA>
        3 <NA>            <NA>             <NA>     <NA>
        4 <NA>            <NA>             <NA>     <NA>
        5 <NA>            <NA>             <NA>     <NA>
        6 <NA>            <NA>             <NA>     <NA>
        7 <NA>            <NA>             <NA>     <NA>
        8 <NA>            <NA>             <NA>     <NA>
        9 <NA>            <NA>             <NA>     <NA>
       10 <NA>            <NA>             <NA>     <NA>
       11 <NA>            <NA>             <NA>     <NA>
       12 <NA>            <NA>             <NA>     <NA>
       13 <NA>            <NA>             <NA>     <NA>
       14 <NA>            <NA>             <NA>     <NA>
       15 <NA>            <NA>             <NA>     <NA>
       16 <NA>            <NA>             <NA>     <NA>
       17 <NA>            <NA>             <NA>     <NA>
       18 <NA>            <NA>             <NA>     <NA>
       19 <NA>            <NA>             <NA>     <NA>
       20 <NA>            <NA>             <NA>     <NA>
       21 <NA>            <NA>             <NA>     <NA>
       22 <NA>            <NA>             <NA>     <NA>
       23 <NA>            <NA>             <NA>     <NA>
       24 <NA>            <NA>             <NA>     <NA>
       25 <NA>            <NA>             <NA>     <NA>
       26 <NA>            <NA>             <NA>     <NA>
       27 <NA>            <NA>             <NA>     <NA>
       28 <NA>            <NA>             <NA>     <NA>
       29 <NA>            <NA>             <NA>     <NA>
       30 <NA>            <NA>             <NA>     <NA>
       31 <NA>            <NA>             <NA>     <NA>
       32 year            catch            interval 0002-01-01
       33 year            catch            interval 0002-01-01
       34 year            catch            interval 0002-01-01
       35 year            catch            interval 0002-01-01
       36 year            catch            interval 0002-01-01
       37 year            catch            interval 0002-01-01
       38 year            catch            interval 0002-01-01
       39 year            catch            interval 0002-01-01
       40 year            catch            interval 0002-01-01
       41 year            catch            interval 0002-01-01
       42 year            catch            interval 0002-01-01
       43 year            catch            interval 0002-01-01
       44 year            catch            interval 0003-01-01
       45 year            catch            interval 0003-01-01
       46 year            catch            interval 0003-01-01
       47 year            catch            interval 0003-01-01
       48 year            catch            interval 0003-01-01
       49 year            catch            interval 0003-01-01
       50 year            catch            interval 0003-01-01
       51 year            catch            interval 0003-01-01
       52 year            catch            interval 0003-01-01
       53 year            catch            interval 0003-01-01
       54 year            catch            interval 0003-01-01
       55 year            catch            interval 0003-01-01
       56 year            catch            interval 0004-01-01
       57 year            catch            interval 0004-01-01
       58 year            catch            interval 0004-01-01
       59 year            catch            interval 0004-01-01
       60 year            catch            interval 0004-01-01
       61 year            catch            interval 0004-01-01
       62 year            catch            interval 0004-01-01
       63 year            catch            interval 0004-01-01
       64 year            catch            interval 0004-01-01
       65 year            catch            interval 0004-01-01
       66 year            catch            interval 0004-01-01
       67 year            catch            interval 0004-01-01
       68 year            catch            interval 0005-01-01
       69 year            catch            interval 0005-01-01
       70 year            catch            interval 0005-01-01
       71 year            catch            interval 0005-01-01
       72 year            catch            interval 0005-01-01
       73 year            catch            interval 0005-01-01
       74 year            catch            interval 0005-01-01
       75 year            catch            interval 0005-01-01
       76 year            catch            interval 0005-01-01
       77 year            catch            interval 0005-01-01
       78 year            catch            interval 0005-01-01
       79 year            catch            interval 0005-01-01
       80 year            catch            interval 0006-01-01
       81 year            catch            interval 0006-01-01
       82 year            catch            interval 0006-01-01
       83 year            catch            interval 0006-01-01
       84 year            catch            interval 0006-01-01
       85 year            catch            interval 0006-01-01
       86 year            catch            interval 0006-01-01
       87 year            catch            interval 0006-01-01
       88 year            catch            interval 0006-01-01
       89 year            catch            interval 0006-01-01
       90 year            catch            interval 0006-01-01
       91 year            catch            interval 0006-01-01
       92 year            catch            interval 0007-01-01
       93 year            catch            interval 0007-01-01
       94 year            catch            interval 0007-01-01
       95 year            catch            interval 0007-01-01
       96 year            catch            interval 0007-01-01
       97 year            catch            interval 0007-01-01
       98 year            catch            interval 0007-01-01
       99 year            catch            interval 0007-01-01
      100 year            catch            interval 0007-01-01
      101 year            catch            interval 0007-01-01
      102 year            catch            interval 0007-01-01
      103 year            catch            interval 0007-01-01
      104 year            catch            interval 0008-01-01
      105 year            catch            interval 0008-01-01
      106 year            catch            interval 0008-01-01
      107 year            catch            interval 0008-01-01
      108 year            catch            interval 0008-01-01
      109 year            catch            interval 0008-01-01
      110 year            catch            interval 0008-01-01
      111 year            catch            interval 0008-01-01
      112 year            catch            interval 0008-01-01
      113 year            catch            interval 0008-01-01
      114 year            catch            interval 0008-01-01
      115 year            catch            interval 0008-01-01
      116 year            catch            interval 0009-01-01
      117 year            catch            interval 0009-01-01
      118 year            catch            interval 0009-01-01
      119 year            catch            interval 0009-01-01
      120 year            catch            interval 0009-01-01
      121 year            catch            interval 0009-01-01
      122 year            catch            interval 0009-01-01
      123 year            catch            interval 0009-01-01
      124 year            catch            interval 0009-01-01
      125 year            catch            interval 0009-01-01
      126 year            catch            interval 0009-01-01
      127 year            catch            interval 0009-01-01
      128 year            catch            interval 0010-01-01
      129 year            catch            interval 0010-01-01
      130 year            catch            interval 0010-01-01
      131 year            catch            interval 0010-01-01
      132 year            catch            interval 0010-01-01
      133 year            catch            interval 0010-01-01
      134 year            catch            interval 0010-01-01
      135 year            catch            interval 0010-01-01
      136 year            catch            interval 0010-01-01
      137 year            catch            interval 0010-01-01
      138 year            catch            interval 0010-01-01
      139 year            catch            interval 0010-01-01
      140 year            catch            interval 0011-01-01
      141 year            catch            interval 0011-01-01
      142 year            catch            interval 0011-01-01
      143 year            catch            interval 0011-01-01
      144 year            catch            interval 0011-01-01
      145 year            catch            interval 0011-01-01
      146 year            catch            interval 0011-01-01
      147 year            catch            interval 0011-01-01
      148 year            catch            interval 0011-01-01
      149 year            catch            interval 0011-01-01
      150 year            catch            interval 0011-01-01
      151 year            catch            interval 0011-01-01
      152 year            catch            interval 0012-01-01
      153 year            catch            interval 0012-01-01
      154 year            catch            interval 0012-01-01
      155 year            catch            interval 0012-01-01
      156 year            catch            interval 0012-01-01
      157 year            catch            interval 0012-01-01
      158 year            catch            interval 0012-01-01
      159 year            catch            interval 0012-01-01
      160 year            catch            interval 0012-01-01
      161 year            catch            interval 0012-01-01
      162 year            catch            interval 0012-01-01
      163 year            catch            interval 0012-01-01
      164 year            catch            interval 0013-01-01
      165 year            catch            interval 0013-01-01
      166 year            catch            interval 0013-01-01
      167 year            catch            interval 0013-01-01
      168 year            catch            interval 0013-01-01
      169 year            catch            interval 0013-01-01
      170 year            catch            interval 0013-01-01
      171 year            catch            interval 0013-01-01
      172 year            catch            interval 0013-01-01
      173 year            catch            interval 0013-01-01
      174 year            catch            interval 0013-01-01
      175 year            catch            interval 0013-01-01
      176 year            catch            interval 0014-01-01
      177 year            catch            interval 0014-01-01
      178 year            catch            interval 0014-01-01
      179 year            catch            interval 0014-01-01
      180 year            catch            interval 0014-01-01
      181 year            catch            interval 0014-01-01
      182 year            catch            interval 0014-01-01
      183 year            catch            interval 0014-01-01
      184 year            catch            interval 0014-01-01
      185 year            catch            interval 0014-01-01
      186 year            catch            interval 0014-01-01
      187 year            catch            interval 0014-01-01
      188 year            catch            interval 0015-01-01
      189 year            catch            interval 0015-01-01
      190 year            catch            interval 0015-01-01
      191 year            catch            interval 0015-01-01
      192 year            catch            interval 0015-01-01
      193 year            catch            interval 0015-01-01
      194 year            catch            interval 0015-01-01
      195 year            catch            interval 0015-01-01
      196 year            catch            interval 0015-01-01
      197 year            catch            interval 0015-01-01
      198 year            catch            interval 0015-01-01
      199 year            catch            interval 0015-01-01
      200 year            catch            interval 0016-01-01
      201 year            catch            interval 0016-01-01
      202 year            catch            interval 0016-01-01
      203 year            catch            interval 0016-01-01
      204 year            catch            interval 0016-01-01
      205 year            catch            interval 0016-01-01
      206 year            catch            interval 0016-01-01
      207 year            catch            interval 0016-01-01
      208 year            catch            interval 0016-01-01
      209 year            catch            interval 0016-01-01
      210 year            catch            interval 0016-01-01
      211 year            catch            interval 0016-01-01
      212 year            catch            interval 0017-01-01
      213 year            catch            interval 0017-01-01
      214 year            catch            interval 0017-01-01
      215 year            catch            interval 0017-01-01
      216 year            catch            interval 0017-01-01
      217 year            catch            interval 0017-01-01
      218 year            catch            interval 0017-01-01
      219 year            catch            interval 0017-01-01
      220 year            catch            interval 0017-01-01
      221 year            catch            interval 0017-01-01
      222 year            catch            interval 0017-01-01
      223 year            catch            interval 0017-01-01
      224 year            catch            interval 0018-01-01
      225 year            catch            interval 0018-01-01
      226 year            catch            interval 0018-01-01
      227 year            catch            interval 0018-01-01
      228 year            catch            interval 0018-01-01
      229 year            catch            interval 0018-01-01
      230 year            catch            interval 0018-01-01
      231 year            catch            interval 0018-01-01
      232 year            catch            interval 0018-01-01
      233 year            catch            interval 0018-01-01
      234 year            catch            interval 0018-01-01
      235 year            catch            interval 0018-01-01
      236 year            catch            interval 0019-01-01
      237 year            catch            interval 0019-01-01
      238 year            catch            interval 0019-01-01
      239 year            catch            interval 0019-01-01
      240 year            catch            interval 0019-01-01
      241 year            catch            interval 0019-01-01
      242 year            catch            interval 0019-01-01
      243 year            catch            interval 0019-01-01
      244 year            catch            interval 0019-01-01
      245 year            catch            interval 0019-01-01
      246 year            catch            interval 0019-01-01
      247 year            catch            interval 0019-01-01
      248 year            catch            interval 0020-01-01
      249 year            catch            interval 0020-01-01
      250 year            catch            interval 0020-01-01
      251 year            catch            interval 0020-01-01
      252 year            catch            interval 0020-01-01
      253 year            catch            interval 0020-01-01
      254 year            catch            interval 0020-01-01
      255 year            catch            interval 0020-01-01
      256 year            catch            interval 0020-01-01
      257 year            catch            interval 0020-01-01
      258 year            catch            interval 0020-01-01
      259 year            catch            interval 0020-01-01
      260 year            catch            interval 0021-01-01
      261 year            catch            interval 0021-01-01
      262 year            catch            interval 0021-01-01
      263 year            catch            interval 0021-01-01
      264 year            catch            interval 0021-01-01
      265 year            catch            interval 0021-01-01
      266 year            catch            interval 0021-01-01
      267 year            catch            interval 0021-01-01
      268 year            catch            interval 0021-01-01
      269 year            catch            interval 0021-01-01
      270 year            catch            interval 0021-01-01
      271 year            catch            interval 0021-01-01
      272 year            catch            interval 0022-01-01
      273 year            catch            interval 0022-01-01
      274 year            catch            interval 0022-01-01
      275 year            catch            interval 0022-01-01
      276 year            catch            interval 0022-01-01
      277 year            catch            interval 0022-01-01
      278 year            catch            interval 0022-01-01
      279 year            catch            interval 0022-01-01
      280 year            catch            interval 0022-01-01
      281 year            catch            interval 0022-01-01
      282 year            catch            interval 0022-01-01
      283 year            catch            interval 0022-01-01
      284 year            catch            interval 0023-01-01
      285 year            catch            interval 0023-01-01
      286 year            catch            interval 0023-01-01
      287 year            catch            interval 0023-01-01
      288 year            catch            interval 0023-01-01
      289 year            catch            interval 0023-01-01
      290 year            catch            interval 0023-01-01
      291 year            catch            interval 0023-01-01
      292 year            catch            interval 0023-01-01
      293 year            catch            interval 0023-01-01
      294 year            catch            interval 0023-01-01
      295 year            catch            interval 0023-01-01
      296 year            catch            interval 0024-01-01
      297 year            catch            interval 0024-01-01
      298 year            catch            interval 0024-01-01
      299 year            catch            interval 0024-01-01
      300 year            catch            interval 0024-01-01
      301 year            catch            interval 0024-01-01
      302 year            catch            interval 0024-01-01
      303 year            catch            interval 0024-01-01
      304 year            catch            interval 0024-01-01
      305 year            catch            interval 0024-01-01
      306 year            catch            interval 0024-01-01
      307 year            catch            interval 0024-01-01
      308 year            catch            interval 0025-01-01
      309 year            catch            interval 0025-01-01
      310 year            catch            interval 0025-01-01
      311 year            catch            interval 0025-01-01
      312 year            catch            interval 0025-01-01
      313 year            catch            interval 0025-01-01
      314 year            catch            interval 0025-01-01
      315 year            catch            interval 0025-01-01
      316 year            catch            interval 0025-01-01
      317 year            catch            interval 0025-01-01
      318 year            catch            interval 0025-01-01
      319 year            catch            interval 0025-01-01
      320 year            catch            interval 0026-01-01
          observation_id                            partition population
          <chr>                                     <chr>     <chr>
        1 <NA>                                      <NA>      <NA>
        2 <NA>                                      <NA>      <NA>
        3 <NA>                                      <NA>      <NA>
        4 <NA>                                      <NA>      <NA>
        5 <NA>                                      <NA>      <NA>
        6 <NA>                                      <NA>      <NA>
        7 <NA>                                      <NA>      <NA>
        8 <NA>                                      <NA>      <NA>
        9 <NA>                                      <NA>      <NA>
       10 <NA>                                      <NA>      <NA>
       11 <NA>                                      <NA>      <NA>
       12 <NA>                                      <NA>      <NA>
       13 <NA>                                      <NA>      <NA>
       14 <NA>                                      <NA>      <NA>
       15 <NA>                                      <NA>      <NA>
       16 <NA>                                      <NA>      <NA>
       17 <NA>                                      <NA>      <NA>
       18 <NA>                                      <NA>      <NA>
       19 <NA>                                      <NA>      <NA>
       20 <NA>                                      <NA>      <NA>
       21 <NA>                                      <NA>      <NA>
       22 <NA>                                      <NA>      <NA>
       23 <NA>                                      <NA>      <NA>
       24 <NA>                                      <NA>      <NA>
       25 <NA>                                      <NA>      <NA>
       26 <NA>                                      <NA>      <NA>
       27 <NA>                                      <NA>      <NA>
       28 <NA>                                      <NA>      <NA>
       29 <NA>                                      <NA>      <NA>
       30 <NA>                                      <NA>      <NA>
       31 <NA>                                      <NA>      <NA>
       32 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       33 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       34 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       35 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       36 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       37 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       38 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       39 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       40 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       41 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       42 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       43 6:fleet1|8:age_comp|10:0001-01-01|5:catch pooled    population1
       44 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       45 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       46 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       47 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       48 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       49 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       50 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       51 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       52 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       53 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       54 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       55 6:fleet1|8:age_comp|10:0002-01-01|5:catch pooled    population1
       56 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       57 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       58 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       59 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       60 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       61 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       62 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       63 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       64 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       65 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       66 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       67 6:fleet1|8:age_comp|10:0003-01-01|5:catch pooled    population1
       68 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       69 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       70 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       71 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       72 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       73 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       74 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       75 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       76 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       77 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       78 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       79 6:fleet1|8:age_comp|10:0004-01-01|5:catch pooled    population1
       80 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       81 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       82 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       83 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       84 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       85 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       86 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       87 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       88 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       89 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       90 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       91 6:fleet1|8:age_comp|10:0005-01-01|5:catch pooled    population1
       92 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       93 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       94 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       95 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       96 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       97 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       98 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
       99 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      100 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      101 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      102 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      103 6:fleet1|8:age_comp|10:0006-01-01|5:catch pooled    population1
      104 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      105 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      106 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      107 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      108 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      109 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      110 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      111 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      112 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      113 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      114 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      115 6:fleet1|8:age_comp|10:0007-01-01|5:catch pooled    population1
      116 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      117 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      118 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      119 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      120 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      121 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      122 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      123 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      124 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      125 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      126 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      127 6:fleet1|8:age_comp|10:0008-01-01|5:catch pooled    population1
      128 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      129 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      130 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      131 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      132 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      133 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      134 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      135 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      136 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      137 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      138 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      139 6:fleet1|8:age_comp|10:0009-01-01|5:catch pooled    population1
      140 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      141 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      142 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      143 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      144 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      145 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      146 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      147 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      148 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      149 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      150 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      151 6:fleet1|8:age_comp|10:0010-01-01|5:catch pooled    population1
      152 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      153 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      154 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      155 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      156 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      157 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      158 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      159 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      160 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      161 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      162 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      163 6:fleet1|8:age_comp|10:0011-01-01|5:catch pooled    population1
      164 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      165 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      166 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      167 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      168 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      169 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      170 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      171 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      172 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      173 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      174 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      175 6:fleet1|8:age_comp|10:0012-01-01|5:catch pooled    population1
      176 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      177 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      178 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      179 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      180 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      181 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      182 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      183 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      184 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      185 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      186 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      187 6:fleet1|8:age_comp|10:0013-01-01|5:catch pooled    population1
      188 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      189 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      190 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      191 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      192 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      193 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      194 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      195 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      196 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      197 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      198 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      199 6:fleet1|8:age_comp|10:0014-01-01|5:catch pooled    population1
      200 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      201 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      202 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      203 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      204 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      205 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      206 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      207 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      208 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      209 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      210 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      211 6:fleet1|8:age_comp|10:0015-01-01|5:catch pooled    population1
      212 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      213 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      214 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      215 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      216 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      217 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      218 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      219 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      220 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      221 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      222 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      223 6:fleet1|8:age_comp|10:0016-01-01|5:catch pooled    population1
      224 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      225 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      226 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      227 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      228 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      229 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      230 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      231 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      232 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      233 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      234 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      235 6:fleet1|8:age_comp|10:0017-01-01|5:catch pooled    population1
      236 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      237 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      238 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      239 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      240 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      241 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      242 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      243 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      244 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      245 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      246 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      247 6:fleet1|8:age_comp|10:0018-01-01|5:catch pooled    population1
      248 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      249 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      250 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      251 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      252 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      253 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      254 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      255 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      256 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      257 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      258 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      259 6:fleet1|8:age_comp|10:0019-01-01|5:catch pooled    population1
      260 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      261 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      262 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      263 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      264 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      265 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      266 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      267 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      268 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      269 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      270 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      271 6:fleet1|8:age_comp|10:0020-01-01|5:catch pooled    population1
      272 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      273 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      274 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      275 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      276 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      277 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      278 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      279 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      280 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      281 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      282 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      283 6:fleet1|8:age_comp|10:0021-01-01|5:catch pooled    population1
      284 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      285 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      286 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      287 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      288 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      289 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      290 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      291 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      292 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      293 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      294 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      295 6:fleet1|8:age_comp|10:0022-01-01|5:catch pooled    population1
      296 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      297 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      298 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      299 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      300 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      301 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      302 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      303 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      304 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      305 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      306 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      307 6:fleet1|8:age_comp|10:0023-01-01|5:catch pooled    population1
      308 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      309 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      310 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      311 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      312 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      313 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      314 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      315 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      316 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      317 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      318 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      319 6:fleet1|8:age_comp|10:0024-01-01|5:catch pooled    population1
      320 6:fleet1|8:age_comp|10:0025-01-01|5:catch pooled    population1
          year_fraction   day time_id weight_timing length_mapping_timing
                  <dbl> <int>   <int> <chr>         <chr>
        1            NA    NA      NA <NA>          <NA>
        2            NA    NA      NA <NA>          <NA>
        3            NA    NA      NA <NA>          <NA>
        4            NA    NA      NA <NA>          <NA>
        5            NA    NA      NA <NA>          <NA>
        6            NA    NA      NA <NA>          <NA>
        7            NA    NA      NA <NA>          <NA>
        8            NA    NA      NA <NA>          <NA>
        9            NA    NA      NA <NA>          <NA>
       10            NA    NA      NA <NA>          <NA>
       11            NA    NA      NA <NA>          <NA>
       12            NA    NA      NA <NA>          <NA>
       13            NA    NA      NA <NA>          <NA>
       14            NA    NA      NA <NA>          <NA>
       15            NA    NA      NA <NA>          <NA>
       16            NA    NA      NA <NA>          <NA>
       17            NA    NA      NA <NA>          <NA>
       18            NA    NA      NA <NA>          <NA>
       19            NA    NA      NA <NA>          <NA>
       20            NA    NA      NA <NA>          <NA>
       21            NA    NA      NA <NA>          <NA>
       22            NA    NA      NA <NA>          <NA>
       23            NA    NA      NA <NA>          <NA>
       24            NA    NA      NA <NA>          <NA>
       25            NA    NA      NA <NA>          <NA>
       26            NA    NA      NA <NA>          <NA>
       27            NA    NA      NA <NA>          <NA>
       28            NA    NA      NA <NA>          <NA>
       29            NA    NA      NA <NA>          <NA>
       30            NA    NA      NA <NA>          <NA>
       31            NA    NA      NA <NA>          <NA>
       32             0     0       0 annual_lookup fixed_annual
       33             0     0       0 annual_lookup fixed_annual
       34             0     0       0 annual_lookup fixed_annual
       35             0     0       0 annual_lookup fixed_annual
       36             0     0       0 annual_lookup fixed_annual
       37             0     0       0 annual_lookup fixed_annual
       38             0     0       0 annual_lookup fixed_annual
       39             0     0       0 annual_lookup fixed_annual
       40             0     0       0 annual_lookup fixed_annual
       41             0     0       0 annual_lookup fixed_annual
       42             0     0       0 annual_lookup fixed_annual
       43             0     0       0 annual_lookup fixed_annual
       44             0   365       1 annual_lookup fixed_annual
       45             0   365       1 annual_lookup fixed_annual
       46             0   365       1 annual_lookup fixed_annual
       47             0   365       1 annual_lookup fixed_annual
       48             0   365       1 annual_lookup fixed_annual
       49             0   365       1 annual_lookup fixed_annual
       50             0   365       1 annual_lookup fixed_annual
       51             0   365       1 annual_lookup fixed_annual
       52             0   365       1 annual_lookup fixed_annual
       53             0   365       1 annual_lookup fixed_annual
       54             0   365       1 annual_lookup fixed_annual
       55             0   365       1 annual_lookup fixed_annual
       56             0   730       2 annual_lookup fixed_annual
       57             0   730       2 annual_lookup fixed_annual
       58             0   730       2 annual_lookup fixed_annual
       59             0   730       2 annual_lookup fixed_annual
       60             0   730       2 annual_lookup fixed_annual
       61             0   730       2 annual_lookup fixed_annual
       62             0   730       2 annual_lookup fixed_annual
       63             0   730       2 annual_lookup fixed_annual
       64             0   730       2 annual_lookup fixed_annual
       65             0   730       2 annual_lookup fixed_annual
       66             0   730       2 annual_lookup fixed_annual
       67             0   730       2 annual_lookup fixed_annual
       68             0  1095       3 annual_lookup fixed_annual
       69             0  1095       3 annual_lookup fixed_annual
       70             0  1095       3 annual_lookup fixed_annual
       71             0  1095       3 annual_lookup fixed_annual
       72             0  1095       3 annual_lookup fixed_annual
       73             0  1095       3 annual_lookup fixed_annual
       74             0  1095       3 annual_lookup fixed_annual
       75             0  1095       3 annual_lookup fixed_annual
       76             0  1095       3 annual_lookup fixed_annual
       77             0  1095       3 annual_lookup fixed_annual
       78             0  1095       3 annual_lookup fixed_annual
       79             0  1095       3 annual_lookup fixed_annual
       80             0  1461       4 annual_lookup fixed_annual
       81             0  1461       4 annual_lookup fixed_annual
       82             0  1461       4 annual_lookup fixed_annual
       83             0  1461       4 annual_lookup fixed_annual
       84             0  1461       4 annual_lookup fixed_annual
       85             0  1461       4 annual_lookup fixed_annual
       86             0  1461       4 annual_lookup fixed_annual
       87             0  1461       4 annual_lookup fixed_annual
       88             0  1461       4 annual_lookup fixed_annual
       89             0  1461       4 annual_lookup fixed_annual
       90             0  1461       4 annual_lookup fixed_annual
       91             0  1461       4 annual_lookup fixed_annual
       92             0  1826       5 annual_lookup fixed_annual
       93             0  1826       5 annual_lookup fixed_annual
       94             0  1826       5 annual_lookup fixed_annual
       95             0  1826       5 annual_lookup fixed_annual
       96             0  1826       5 annual_lookup fixed_annual
       97             0  1826       5 annual_lookup fixed_annual
       98             0  1826       5 annual_lookup fixed_annual
       99             0  1826       5 annual_lookup fixed_annual
      100             0  1826       5 annual_lookup fixed_annual
      101             0  1826       5 annual_lookup fixed_annual
      102             0  1826       5 annual_lookup fixed_annual
      103             0  1826       5 annual_lookup fixed_annual
      104             0  2191       6 annual_lookup fixed_annual
      105             0  2191       6 annual_lookup fixed_annual
      106             0  2191       6 annual_lookup fixed_annual
      107             0  2191       6 annual_lookup fixed_annual
      108             0  2191       6 annual_lookup fixed_annual
      109             0  2191       6 annual_lookup fixed_annual
      110             0  2191       6 annual_lookup fixed_annual
      111             0  2191       6 annual_lookup fixed_annual
      112             0  2191       6 annual_lookup fixed_annual
      113             0  2191       6 annual_lookup fixed_annual
      114             0  2191       6 annual_lookup fixed_annual
      115             0  2191       6 annual_lookup fixed_annual
      116             0  2556       7 annual_lookup fixed_annual
      117             0  2556       7 annual_lookup fixed_annual
      118             0  2556       7 annual_lookup fixed_annual
      119             0  2556       7 annual_lookup fixed_annual
      120             0  2556       7 annual_lookup fixed_annual
      121             0  2556       7 annual_lookup fixed_annual
      122             0  2556       7 annual_lookup fixed_annual
      123             0  2556       7 annual_lookup fixed_annual
      124             0  2556       7 annual_lookup fixed_annual
      125             0  2556       7 annual_lookup fixed_annual
      126             0  2556       7 annual_lookup fixed_annual
      127             0  2556       7 annual_lookup fixed_annual
      128             0  2922       8 annual_lookup fixed_annual
      129             0  2922       8 annual_lookup fixed_annual
      130             0  2922       8 annual_lookup fixed_annual
      131             0  2922       8 annual_lookup fixed_annual
      132             0  2922       8 annual_lookup fixed_annual
      133             0  2922       8 annual_lookup fixed_annual
      134             0  2922       8 annual_lookup fixed_annual
      135             0  2922       8 annual_lookup fixed_annual
      136             0  2922       8 annual_lookup fixed_annual
      137             0  2922       8 annual_lookup fixed_annual
      138             0  2922       8 annual_lookup fixed_annual
      139             0  2922       8 annual_lookup fixed_annual
      140             0  3287       9 annual_lookup fixed_annual
      141             0  3287       9 annual_lookup fixed_annual
      142             0  3287       9 annual_lookup fixed_annual
      143             0  3287       9 annual_lookup fixed_annual
      144             0  3287       9 annual_lookup fixed_annual
      145             0  3287       9 annual_lookup fixed_annual
      146             0  3287       9 annual_lookup fixed_annual
      147             0  3287       9 annual_lookup fixed_annual
      148             0  3287       9 annual_lookup fixed_annual
      149             0  3287       9 annual_lookup fixed_annual
      150             0  3287       9 annual_lookup fixed_annual
      151             0  3287       9 annual_lookup fixed_annual
      152             0  3652      10 annual_lookup fixed_annual
      153             0  3652      10 annual_lookup fixed_annual
      154             0  3652      10 annual_lookup fixed_annual
      155             0  3652      10 annual_lookup fixed_annual
      156             0  3652      10 annual_lookup fixed_annual
      157             0  3652      10 annual_lookup fixed_annual
      158             0  3652      10 annual_lookup fixed_annual
      159             0  3652      10 annual_lookup fixed_annual
      160             0  3652      10 annual_lookup fixed_annual
      161             0  3652      10 annual_lookup fixed_annual
      162             0  3652      10 annual_lookup fixed_annual
      163             0  3652      10 annual_lookup fixed_annual
      164             0  4017      11 annual_lookup fixed_annual
      165             0  4017      11 annual_lookup fixed_annual
      166             0  4017      11 annual_lookup fixed_annual
      167             0  4017      11 annual_lookup fixed_annual
      168             0  4017      11 annual_lookup fixed_annual
      169             0  4017      11 annual_lookup fixed_annual
      170             0  4017      11 annual_lookup fixed_annual
      171             0  4017      11 annual_lookup fixed_annual
      172             0  4017      11 annual_lookup fixed_annual
      173             0  4017      11 annual_lookup fixed_annual
      174             0  4017      11 annual_lookup fixed_annual
      175             0  4017      11 annual_lookup fixed_annual
      176             0  4383      12 annual_lookup fixed_annual
      177             0  4383      12 annual_lookup fixed_annual
      178             0  4383      12 annual_lookup fixed_annual
      179             0  4383      12 annual_lookup fixed_annual
      180             0  4383      12 annual_lookup fixed_annual
      181             0  4383      12 annual_lookup fixed_annual
      182             0  4383      12 annual_lookup fixed_annual
      183             0  4383      12 annual_lookup fixed_annual
      184             0  4383      12 annual_lookup fixed_annual
      185             0  4383      12 annual_lookup fixed_annual
      186             0  4383      12 annual_lookup fixed_annual
      187             0  4383      12 annual_lookup fixed_annual
      188             0  4748      13 annual_lookup fixed_annual
      189             0  4748      13 annual_lookup fixed_annual
      190             0  4748      13 annual_lookup fixed_annual
      191             0  4748      13 annual_lookup fixed_annual
      192             0  4748      13 annual_lookup fixed_annual
      193             0  4748      13 annual_lookup fixed_annual
      194             0  4748      13 annual_lookup fixed_annual
      195             0  4748      13 annual_lookup fixed_annual
      196             0  4748      13 annual_lookup fixed_annual
      197             0  4748      13 annual_lookup fixed_annual
      198             0  4748      13 annual_lookup fixed_annual
      199             0  4748      13 annual_lookup fixed_annual
      200             0  5113      14 annual_lookup fixed_annual
      201             0  5113      14 annual_lookup fixed_annual
      202             0  5113      14 annual_lookup fixed_annual
      203             0  5113      14 annual_lookup fixed_annual
      204             0  5113      14 annual_lookup fixed_annual
      205             0  5113      14 annual_lookup fixed_annual
      206             0  5113      14 annual_lookup fixed_annual
      207             0  5113      14 annual_lookup fixed_annual
      208             0  5113      14 annual_lookup fixed_annual
      209             0  5113      14 annual_lookup fixed_annual
      210             0  5113      14 annual_lookup fixed_annual
      211             0  5113      14 annual_lookup fixed_annual
      212             0  5478      15 annual_lookup fixed_annual
      213             0  5478      15 annual_lookup fixed_annual
      214             0  5478      15 annual_lookup fixed_annual
      215             0  5478      15 annual_lookup fixed_annual
      216             0  5478      15 annual_lookup fixed_annual
      217             0  5478      15 annual_lookup fixed_annual
      218             0  5478      15 annual_lookup fixed_annual
      219             0  5478      15 annual_lookup fixed_annual
      220             0  5478      15 annual_lookup fixed_annual
      221             0  5478      15 annual_lookup fixed_annual
      222             0  5478      15 annual_lookup fixed_annual
      223             0  5478      15 annual_lookup fixed_annual
      224             0  5844      16 annual_lookup fixed_annual
      225             0  5844      16 annual_lookup fixed_annual
      226             0  5844      16 annual_lookup fixed_annual
      227             0  5844      16 annual_lookup fixed_annual
      228             0  5844      16 annual_lookup fixed_annual
      229             0  5844      16 annual_lookup fixed_annual
      230             0  5844      16 annual_lookup fixed_annual
      231             0  5844      16 annual_lookup fixed_annual
      232             0  5844      16 annual_lookup fixed_annual
      233             0  5844      16 annual_lookup fixed_annual
      234             0  5844      16 annual_lookup fixed_annual
      235             0  5844      16 annual_lookup fixed_annual
      236             0  6209      17 annual_lookup fixed_annual
      237             0  6209      17 annual_lookup fixed_annual
      238             0  6209      17 annual_lookup fixed_annual
      239             0  6209      17 annual_lookup fixed_annual
      240             0  6209      17 annual_lookup fixed_annual
      241             0  6209      17 annual_lookup fixed_annual
      242             0  6209      17 annual_lookup fixed_annual
      243             0  6209      17 annual_lookup fixed_annual
      244             0  6209      17 annual_lookup fixed_annual
      245             0  6209      17 annual_lookup fixed_annual
      246             0  6209      17 annual_lookup fixed_annual
      247             0  6209      17 annual_lookup fixed_annual
      248             0  6574      18 annual_lookup fixed_annual
      249             0  6574      18 annual_lookup fixed_annual
      250             0  6574      18 annual_lookup fixed_annual
      251             0  6574      18 annual_lookup fixed_annual
      252             0  6574      18 annual_lookup fixed_annual
      253             0  6574      18 annual_lookup fixed_annual
      254             0  6574      18 annual_lookup fixed_annual
      255             0  6574      18 annual_lookup fixed_annual
      256             0  6574      18 annual_lookup fixed_annual
      257             0  6574      18 annual_lookup fixed_annual
      258             0  6574      18 annual_lookup fixed_annual
      259             0  6574      18 annual_lookup fixed_annual
      260             0  6939      19 annual_lookup fixed_annual
      261             0  6939      19 annual_lookup fixed_annual
      262             0  6939      19 annual_lookup fixed_annual
      263             0  6939      19 annual_lookup fixed_annual
      264             0  6939      19 annual_lookup fixed_annual
      265             0  6939      19 annual_lookup fixed_annual
      266             0  6939      19 annual_lookup fixed_annual
      267             0  6939      19 annual_lookup fixed_annual
      268             0  6939      19 annual_lookup fixed_annual
      269             0  6939      19 annual_lookup fixed_annual
      270             0  6939      19 annual_lookup fixed_annual
      271             0  6939      19 annual_lookup fixed_annual
      272             0  7305      20 annual_lookup fixed_annual
      273             0  7305      20 annual_lookup fixed_annual
      274             0  7305      20 annual_lookup fixed_annual
      275             0  7305      20 annual_lookup fixed_annual
      276             0  7305      20 annual_lookup fixed_annual
      277             0  7305      20 annual_lookup fixed_annual
      278             0  7305      20 annual_lookup fixed_annual
      279             0  7305      20 annual_lookup fixed_annual
      280             0  7305      20 annual_lookup fixed_annual
      281             0  7305      20 annual_lookup fixed_annual
      282             0  7305      20 annual_lookup fixed_annual
      283             0  7305      20 annual_lookup fixed_annual
      284             0  7670      21 annual_lookup fixed_annual
      285             0  7670      21 annual_lookup fixed_annual
      286             0  7670      21 annual_lookup fixed_annual
      287             0  7670      21 annual_lookup fixed_annual
      288             0  7670      21 annual_lookup fixed_annual
      289             0  7670      21 annual_lookup fixed_annual
      290             0  7670      21 annual_lookup fixed_annual
      291             0  7670      21 annual_lookup fixed_annual
      292             0  7670      21 annual_lookup fixed_annual
      293             0  7670      21 annual_lookup fixed_annual
      294             0  7670      21 annual_lookup fixed_annual
      295             0  7670      21 annual_lookup fixed_annual
      296             0  8035      22 annual_lookup fixed_annual
      297             0  8035      22 annual_lookup fixed_annual
      298             0  8035      22 annual_lookup fixed_annual
      299             0  8035      22 annual_lookup fixed_annual
      300             0  8035      22 annual_lookup fixed_annual
      301             0  8035      22 annual_lookup fixed_annual
      302             0  8035      22 annual_lookup fixed_annual
      303             0  8035      22 annual_lookup fixed_annual
      304             0  8035      22 annual_lookup fixed_annual
      305             0  8035      22 annual_lookup fixed_annual
      306             0  8035      22 annual_lookup fixed_annual
      307             0  8035      22 annual_lookup fixed_annual
      308             0  8400      23 annual_lookup fixed_annual
      309             0  8400      23 annual_lookup fixed_annual
      310             0  8400      23 annual_lookup fixed_annual
      311             0  8400      23 annual_lookup fixed_annual
      312             0  8400      23 annual_lookup fixed_annual
      313             0  8400      23 annual_lookup fixed_annual
      314             0  8400      23 annual_lookup fixed_annual
      315             0  8400      23 annual_lookup fixed_annual
      316             0  8400      23 annual_lookup fixed_annual
      317             0  8400      23 annual_lookup fixed_annual
      318             0  8400      23 annual_lookup fixed_annual
      319             0  8400      23 annual_lookup fixed_annual
      320             0  8766      24 annual_lookup fixed_annual
          maturity_timing prediction_timing
          <chr>           <chr>
        1 <NA>            <NA>
        2 <NA>            <NA>
        3 <NA>            <NA>
        4 <NA>            <NA>
        5 <NA>            <NA>
        6 <NA>            <NA>
        7 <NA>            <NA>
        8 <NA>            <NA>
        9 <NA>            <NA>
       10 <NA>            <NA>
       11 <NA>            <NA>
       12 <NA>            <NA>
       13 <NA>            <NA>
       14 <NA>            <NA>
       15 <NA>            <NA>
       16 <NA>            <NA>
       17 <NA>            <NA>
       18 <NA>            <NA>
       19 <NA>            <NA>
       20 <NA>            <NA>
       21 <NA>            <NA>
       22 <NA>            <NA>
       23 <NA>            <NA>
       24 <NA>            <NA>
       25 <NA>            <NA>
       26 <NA>            <NA>
       27 <NA>            <NA>
       28 <NA>            <NA>
       29 <NA>            <NA>
       30 <NA>            <NA>
       31 <NA>            <NA>
       32 fractional_age  annual_interval
       33 fractional_age  annual_interval
       34 fractional_age  annual_interval
       35 fractional_age  annual_interval
       36 fractional_age  annual_interval
       37 fractional_age  annual_interval
       38 fractional_age  annual_interval
       39 fractional_age  annual_interval
       40 fractional_age  annual_interval
       41 fractional_age  annual_interval
       42 fractional_age  annual_interval
       43 fractional_age  annual_interval
       44 fractional_age  annual_interval
       45 fractional_age  annual_interval
       46 fractional_age  annual_interval
       47 fractional_age  annual_interval
       48 fractional_age  annual_interval
       49 fractional_age  annual_interval
       50 fractional_age  annual_interval
       51 fractional_age  annual_interval
       52 fractional_age  annual_interval
       53 fractional_age  annual_interval
       54 fractional_age  annual_interval
       55 fractional_age  annual_interval
       56 fractional_age  annual_interval
       57 fractional_age  annual_interval
       58 fractional_age  annual_interval
       59 fractional_age  annual_interval
       60 fractional_age  annual_interval
       61 fractional_age  annual_interval
       62 fractional_age  annual_interval
       63 fractional_age  annual_interval
       64 fractional_age  annual_interval
       65 fractional_age  annual_interval
       66 fractional_age  annual_interval
       67 fractional_age  annual_interval
       68 fractional_age  annual_interval
       69 fractional_age  annual_interval
       70 fractional_age  annual_interval
       71 fractional_age  annual_interval
       72 fractional_age  annual_interval
       73 fractional_age  annual_interval
       74 fractional_age  annual_interval
       75 fractional_age  annual_interval
       76 fractional_age  annual_interval
       77 fractional_age  annual_interval
       78 fractional_age  annual_interval
       79 fractional_age  annual_interval
       80 fractional_age  annual_interval
       81 fractional_age  annual_interval
       82 fractional_age  annual_interval
       83 fractional_age  annual_interval
       84 fractional_age  annual_interval
       85 fractional_age  annual_interval
       86 fractional_age  annual_interval
       87 fractional_age  annual_interval
       88 fractional_age  annual_interval
       89 fractional_age  annual_interval
       90 fractional_age  annual_interval
       91 fractional_age  annual_interval
       92 fractional_age  annual_interval
       93 fractional_age  annual_interval
       94 fractional_age  annual_interval
       95 fractional_age  annual_interval
       96 fractional_age  annual_interval
       97 fractional_age  annual_interval
       98 fractional_age  annual_interval
       99 fractional_age  annual_interval
      100 fractional_age  annual_interval
      101 fractional_age  annual_interval
      102 fractional_age  annual_interval
      103 fractional_age  annual_interval
      104 fractional_age  annual_interval
      105 fractional_age  annual_interval
      106 fractional_age  annual_interval
      107 fractional_age  annual_interval
      108 fractional_age  annual_interval
      109 fractional_age  annual_interval
      110 fractional_age  annual_interval
      111 fractional_age  annual_interval
      112 fractional_age  annual_interval
      113 fractional_age  annual_interval
      114 fractional_age  annual_interval
      115 fractional_age  annual_interval
      116 fractional_age  annual_interval
      117 fractional_age  annual_interval
      118 fractional_age  annual_interval
      119 fractional_age  annual_interval
      120 fractional_age  annual_interval
      121 fractional_age  annual_interval
      122 fractional_age  annual_interval
      123 fractional_age  annual_interval
      124 fractional_age  annual_interval
      125 fractional_age  annual_interval
      126 fractional_age  annual_interval
      127 fractional_age  annual_interval
      128 fractional_age  annual_interval
      129 fractional_age  annual_interval
      130 fractional_age  annual_interval
      131 fractional_age  annual_interval
      132 fractional_age  annual_interval
      133 fractional_age  annual_interval
      134 fractional_age  annual_interval
      135 fractional_age  annual_interval
      136 fractional_age  annual_interval
      137 fractional_age  annual_interval
      138 fractional_age  annual_interval
      139 fractional_age  annual_interval
      140 fractional_age  annual_interval
      141 fractional_age  annual_interval
      142 fractional_age  annual_interval
      143 fractional_age  annual_interval
      144 fractional_age  annual_interval
      145 fractional_age  annual_interval
      146 fractional_age  annual_interval
      147 fractional_age  annual_interval
      148 fractional_age  annual_interval
      149 fractional_age  annual_interval
      150 fractional_age  annual_interval
      151 fractional_age  annual_interval
      152 fractional_age  annual_interval
      153 fractional_age  annual_interval
      154 fractional_age  annual_interval
      155 fractional_age  annual_interval
      156 fractional_age  annual_interval
      157 fractional_age  annual_interval
      158 fractional_age  annual_interval
      159 fractional_age  annual_interval
      160 fractional_age  annual_interval
      161 fractional_age  annual_interval
      162 fractional_age  annual_interval
      163 fractional_age  annual_interval
      164 fractional_age  annual_interval
      165 fractional_age  annual_interval
      166 fractional_age  annual_interval
      167 fractional_age  annual_interval
      168 fractional_age  annual_interval
      169 fractional_age  annual_interval
      170 fractional_age  annual_interval
      171 fractional_age  annual_interval
      172 fractional_age  annual_interval
      173 fractional_age  annual_interval
      174 fractional_age  annual_interval
      175 fractional_age  annual_interval
      176 fractional_age  annual_interval
      177 fractional_age  annual_interval
      178 fractional_age  annual_interval
      179 fractional_age  annual_interval
      180 fractional_age  annual_interval
      181 fractional_age  annual_interval
      182 fractional_age  annual_interval
      183 fractional_age  annual_interval
      184 fractional_age  annual_interval
      185 fractional_age  annual_interval
      186 fractional_age  annual_interval
      187 fractional_age  annual_interval
      188 fractional_age  annual_interval
      189 fractional_age  annual_interval
      190 fractional_age  annual_interval
      191 fractional_age  annual_interval
      192 fractional_age  annual_interval
      193 fractional_age  annual_interval
      194 fractional_age  annual_interval
      195 fractional_age  annual_interval
      196 fractional_age  annual_interval
      197 fractional_age  annual_interval
      198 fractional_age  annual_interval
      199 fractional_age  annual_interval
      200 fractional_age  annual_interval
      201 fractional_age  annual_interval
      202 fractional_age  annual_interval
      203 fractional_age  annual_interval
      204 fractional_age  annual_interval
      205 fractional_age  annual_interval
      206 fractional_age  annual_interval
      207 fractional_age  annual_interval
      208 fractional_age  annual_interval
      209 fractional_age  annual_interval
      210 fractional_age  annual_interval
      211 fractional_age  annual_interval
      212 fractional_age  annual_interval
      213 fractional_age  annual_interval
      214 fractional_age  annual_interval
      215 fractional_age  annual_interval
      216 fractional_age  annual_interval
      217 fractional_age  annual_interval
      218 fractional_age  annual_interval
      219 fractional_age  annual_interval
      220 fractional_age  annual_interval
      221 fractional_age  annual_interval
      222 fractional_age  annual_interval
      223 fractional_age  annual_interval
      224 fractional_age  annual_interval
      225 fractional_age  annual_interval
      226 fractional_age  annual_interval
      227 fractional_age  annual_interval
      228 fractional_age  annual_interval
      229 fractional_age  annual_interval
      230 fractional_age  annual_interval
      231 fractional_age  annual_interval
      232 fractional_age  annual_interval
      233 fractional_age  annual_interval
      234 fractional_age  annual_interval
      235 fractional_age  annual_interval
      236 fractional_age  annual_interval
      237 fractional_age  annual_interval
      238 fractional_age  annual_interval
      239 fractional_age  annual_interval
      240 fractional_age  annual_interval
      241 fractional_age  annual_interval
      242 fractional_age  annual_interval
      243 fractional_age  annual_interval
      244 fractional_age  annual_interval
      245 fractional_age  annual_interval
      246 fractional_age  annual_interval
      247 fractional_age  annual_interval
      248 fractional_age  annual_interval
      249 fractional_age  annual_interval
      250 fractional_age  annual_interval
      251 fractional_age  annual_interval
      252 fractional_age  annual_interval
      253 fractional_age  annual_interval
      254 fractional_age  annual_interval
      255 fractional_age  annual_interval
      256 fractional_age  annual_interval
      257 fractional_age  annual_interval
      258 fractional_age  annual_interval
      259 fractional_age  annual_interval
      260 fractional_age  annual_interval
      261 fractional_age  annual_interval
      262 fractional_age  annual_interval
      263 fractional_age  annual_interval
      264 fractional_age  annual_interval
      265 fractional_age  annual_interval
      266 fractional_age  annual_interval
      267 fractional_age  annual_interval
      268 fractional_age  annual_interval
      269 fractional_age  annual_interval
      270 fractional_age  annual_interval
      271 fractional_age  annual_interval
      272 fractional_age  annual_interval
      273 fractional_age  annual_interval
      274 fractional_age  annual_interval
      275 fractional_age  annual_interval
      276 fractional_age  annual_interval
      277 fractional_age  annual_interval
      278 fractional_age  annual_interval
      279 fractional_age  annual_interval
      280 fractional_age  annual_interval
      281 fractional_age  annual_interval
      282 fractional_age  annual_interval
      283 fractional_age  annual_interval
      284 fractional_age  annual_interval
      285 fractional_age  annual_interval
      286 fractional_age  annual_interval
      287 fractional_age  annual_interval
      288 fractional_age  annual_interval
      289 fractional_age  annual_interval
      290 fractional_age  annual_interval
      291 fractional_age  annual_interval
      292 fractional_age  annual_interval
      293 fractional_age  annual_interval
      294 fractional_age  annual_interval
      295 fractional_age  annual_interval
      296 fractional_age  annual_interval
      297 fractional_age  annual_interval
      298 fractional_age  annual_interval
      299 fractional_age  annual_interval
      300 fractional_age  annual_interval
      301 fractional_age  annual_interval
      302 fractional_age  annual_interval
      303 fractional_age  annual_interval
      304 fractional_age  annual_interval
      305 fractional_age  annual_interval
      306 fractional_age  annual_interval
      307 fractional_age  annual_interval
      308 fractional_age  annual_interval
      309 fractional_age  annual_interval
      310 fractional_age  annual_interval
      311 fractional_age  annual_interval
      312 fractional_age  annual_interval
      313 fractional_age  annual_interval
      314 fractional_age  annual_interval
      315 fractional_age  annual_interval
      316 fractional_age  annual_interval
      317 fractional_age  annual_interval
      318 fractional_age  annual_interval
      319 fractional_age  annual_interval
      320 fractional_age  annual_interval
      # i 13,676 more rows

