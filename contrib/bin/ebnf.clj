; Copyright (C) 2024 olang mantainers
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

(require '[clojure.pprint :refer [pprint]])
(require '[instaparse.core :refer [parser]])
(require '[instaparse.failure :refer [pprint-failure]])

(def olang-file (System/getenv "FILE"))

(def olang-parser (parser (slurp *in*)))

(def parser-result (olang-parser (slurp olang-file)))

(defn println-err
  ([ ] (.println *err* ""))
  ([s] (.println *err* s)))

(defn print-err
  ([ ] (.print *err* ""))
  ([s] (.print *err* s)))

(defn main []
  (if (:reason parser-result)
    (with-redefs [clojure.core/println println-err
                  clojure.core/print print-err]
      (pprint-failure parser-result)
      (System/exit 1))
    (pprint parser-result)))

(main)
